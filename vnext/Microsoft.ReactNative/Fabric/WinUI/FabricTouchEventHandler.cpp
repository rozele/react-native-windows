// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <pch.h>

#include "FabricTouchEventHandler.h"

#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <Fabric/WinUI/FabricUIManagerModule.h>
#include <react/renderer/components/view/TouchEventEmitter.h>

#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <Utils/Helpers.h>
#include <Utils/ValueUtils.h>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Devices.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>

#ifdef USE_WINUI3
#include <winrt/Microsoft.UI.Input.h>
namespace input = winrt::Microsoft::UI::Input;
#else
namespace input = winrt::Windows::UI::Input;
#endif

namespace Microsoft::ReactNative {

std::shared_ptr<FabricUIManager> GetFabricUIManager(const Mso::React::IReactContext &context) {
  return FabricUIManager::FromProperties(winrt::Microsoft::ReactNative::ReactPropertyBag(context.Properties()));
}

std::shared_ptr<BaseComponentView const> GetNearestReactViewAncestor(
    std::shared_ptr<FabricUIManager> uiManager,
    xaml::DependencyObject const &element) {
  auto ancestor = element;
  while (ancestor) {
    const auto tag = static_cast<facebook::react::Tag>(GetTag(element));
    if (auto view = std::static_pointer_cast<BaseComponentView const>(
            uiManager->GetViewRegistry().findComponentViewWithTag(tag))) {
      return view;
    } else {
      ancestor = winrt::VisualTreeHelper::GetParent(ancestor).try_as<xaml::UIElement>();
    }
  }

  return nullptr;
}

FabricTouchEventHandler::FabricTouchEventHandler(const Mso::React::IReactContext &context)
    : m_xamlView(nullptr), m_rootView(nullptr), m_context(&context) {}

FabricTouchEventHandler::~FabricTouchEventHandler() {
  RemoveTouchHandlers();
}

void FabricTouchEventHandler::AddTouchHandlers(XamlView xamlView, XamlView rootView, bool handledEventsToo) {
  auto uiElement(xamlView.try_as<xaml::UIElement>());
  if (uiElement == nullptr) {
    assert(false);
    return;
  }

  RemoveTouchHandlers();

  m_xamlView = xamlView;
  m_rootView = rootView != nullptr ? rootView : xamlView;
  m_pressedHandler = winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerPressed});
  m_releasedHandler = winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerReleased});
  m_canceledHandler = winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerCanceled});
  m_captureLostHandler =
      winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerCaptureLost});
  m_exitedHandler = winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerExited});
  m_movedHandler = winrt::box_value(winrt::PointerEventHandler{this, &FabricTouchEventHandler::OnPointerMoved});
  uiElement.AddHandler(xaml::UIElement::PointerPressedEvent(), m_pressedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerReleasedEvent(), m_releasedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerCanceledEvent(), m_canceledHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerCaptureLostEvent(), m_captureLostHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerExitedEvent(), m_exitedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerMovedEvent(), m_movedHandler, handledEventsToo);
}

void FabricTouchEventHandler::RemoveTouchHandlers() {
  if (m_xamlView) {
    auto uiElement(m_xamlView.as<xaml::UIElement>());
    uiElement.RemoveHandler(xaml::UIElement::PointerPressedEvent(), m_pressedHandler);
    uiElement.RemoveHandler(xaml::UIElement::PointerReleasedEvent(), m_releasedHandler);
    uiElement.RemoveHandler(xaml::UIElement::PointerCanceledEvent(), m_canceledHandler);
    uiElement.RemoveHandler(xaml::UIElement::PointerCaptureLostEvent(), m_captureLostHandler);
    uiElement.RemoveHandler(xaml::UIElement::PointerExitedEvent(), m_exitedHandler);
    uiElement.RemoveHandler(xaml::UIElement::PointerMovedEvent(), m_movedHandler);
    m_pressedHandler = nullptr;
    m_releasedHandler = nullptr;
    m_canceledHandler = nullptr;
    m_captureLostHandler = nullptr;
    m_exitedHandler = nullptr;
    m_movedHandler = nullptr;
    m_rootView = nullptr;
    m_xamlView = nullptr;
  }
}

void FabricTouchEventHandler::OnPointerPressed(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  if (m_context->State() == Mso::React::ReactInstanceState::HasError)
    return;

  if (IndexOfPointerWithId(args.Pointer().PointerId()) != std::nullopt) {
    // A pointer with this ID already exists
    assert(false);
    return;
  }

  // Only if the view has a Tag can we process this
  std::vector<int64_t> tagsForBranch;
  xaml::UIElement sourceElement(nullptr);
  const auto eventType = TouchEventType::Start;
  const auto kind = GetPointerEventKind(eventType);
  const auto reactArgs = winrt::make<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(kind, args);
  if (!PropagatePointerEventAndFindReactSourceBranch(reactArgs, &tagsForBranch, &sourceElement))
    return;

  // If this was caused by the user pressing the "back" hardware button, fire that event instead
  if (args.GetCurrentPoint(sourceElement).Properties().PointerUpdateKind() ==
      input::PointerUpdateKind::XButton1Pressed) {
    args.Handled(DispatchBackEvent());
    return;
  }

  if (m_xamlView.as<xaml::FrameworkElement>().CapturePointer(args.Pointer())) {
    assert(!tagsForBranch.empty());
    const auto tag = tagsForBranch.front();

    size_t pointerIndex = AddReactPointer(args, tag, sourceElement);

    DispatchTouchEvent(eventType, pointerIndex, tagsForBranch);
  }
}

void FabricTouchEventHandler::OnPointerReleased(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::End, args);
}

void FabricTouchEventHandler::OnPointerCanceled(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::Cancel, args);
}

void FabricTouchEventHandler::OnPointerCaptureLost(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::CaptureLost, args);
}

void FabricTouchEventHandler::OnPointerExited(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  if (m_context->State() == Mso::React::ReactInstanceState::HasError)
    return;

  std::vector<int64_t> tagsForBranch;
  UpdatePointersInViews(args, nullptr, std::move(tagsForBranch));
}

void FabricTouchEventHandler::OnPointerMoved(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  if (m_context->State() == Mso::React::ReactInstanceState::HasError)
    return;

  // Only if the view has a Tag can we process this
  std::vector<int64_t> tagsForBranch;
  xaml::UIElement sourceElement(nullptr);
  const auto eventType = TouchEventType::Move;
  const auto kind = GetPointerEventKind(eventType);
  const auto reactArgs = winrt::make<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(kind, args);
  const auto hasReactTarget = PropagatePointerEventAndFindReactSourceBranch(reactArgs, &tagsForBranch, &sourceElement);

  const auto optPointerIndex = IndexOfPointerWithId(args.Pointer().PointerId());
  if (optPointerIndex) {
    UpdateReactPointer(
        m_pointers[*optPointerIndex], args, hasReactTarget ? sourceElement : m_rootView.as<xaml::UIElement>());
    DispatchTouchEvent(eventType, *optPointerIndex, tagsForBranch);
  }

  // If we re-introduce onMouseMove to react-native-windows, we should add an
  // argument to ensure we do not emit these events while the pointer is down.
  UpdatePointersInViews(args, sourceElement, std::move(tagsForBranch));
}

void FabricTouchEventHandler::OnPointerConcluded(TouchEventType eventType, const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  if (m_context->State() == Mso::React::ReactInstanceState::HasError)
    return;

  auto optPointerIndex = IndexOfPointerWithId(args.Pointer().PointerId());
  if (!optPointerIndex)
    return;

  // if the view has a Tag, update the pointer info.
  // Regardless of that, ensure we Dispatch & cleanup the pointer
  std::vector<int64_t> tagsForBranch;
  xaml::UIElement sourceElement(nullptr);
  const auto kind = GetPointerEventKind(eventType);
  const auto reactArgs = winrt::make<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(kind, args);
  const auto hasReactTarget = PropagatePointerEventAndFindReactSourceBranch(reactArgs, &tagsForBranch, &sourceElement);
  UpdateReactPointer(
      m_pointers[*optPointerIndex], args, hasReactTarget ? sourceElement : m_rootView.as<xaml::UIElement>());

  // In case a PointerCaptureLost event should be treated as an "end" event,
  // check the ReactPointerEventArgs Kind property before emitting the event.
  const auto adjustedEventType = reactArgs.Kind() == winrt::Microsoft::ReactNative::PointerEventKind::End
      ? TouchEventType::End
      : TouchEventType::Cancel;
  DispatchTouchEvent(adjustedEventType, *optPointerIndex, tagsForBranch);

  m_pointers.erase(cbegin(m_pointers) + *optPointerIndex);
  if (m_pointers.size() == 0)
    m_touchId = 0;

  m_xamlView.as<xaml::FrameworkElement>().ReleasePointerCapture(args.Pointer());
}

size_t FabricTouchEventHandler::AddReactPointer(
    const winrt::PointerRoutedEventArgs &args,
    int64_t tag,
    xaml::UIElement sourceElement) {
  ReactPointer pointer = CreateReactPointer(args, tag, sourceElement);
  m_pointers.emplace_back(std::move(pointer));
  return m_pointers.size() - 1;
}

FabricTouchEventHandler::ReactPointer FabricTouchEventHandler::CreateReactPointer(
    const winrt::PointerRoutedEventArgs &args,
    int64_t tag,
    xaml::UIElement sourceElement) {
  auto point = args.GetCurrentPoint(sourceElement);
  auto props = point.Properties();

  ReactPointer pointer{};
  pointer.target = tag;
  pointer.identifier = m_touchId++;
  pointer.pointerId = point.PointerId();
#ifndef USE_WINUI3
  pointer.deviceType = point.PointerDevice().PointerDeviceType();
#else
  pointer.deviceType = point.PointerDeviceType();
#endif
  pointer.isLeftButton = props.IsLeftButtonPressed();
  pointer.isRightButton = props.IsRightButtonPressed();
  pointer.isMiddleButton = props.IsMiddleButtonPressed();
  pointer.isHorizontalScrollWheel = props.IsHorizontalMouseWheel();
  pointer.isEraser = props.IsEraser();

  UpdateReactPointer(pointer, args, sourceElement);

  return pointer;
}

void FabricTouchEventHandler::UpdateReactPointer(
    ReactPointer &pointer,
    const winrt::PointerRoutedEventArgs &args,
    xaml::UIElement sourceElement) {
  auto rootPoint = args.GetCurrentPoint(m_rootView.as<xaml::FrameworkElement>());
  auto point = args.GetCurrentPoint(sourceElement);
  auto props = point.Properties();
  auto keyModifiers = static_cast<uint32_t>(args.KeyModifiers());

  pointer.positionRoot = rootPoint.Position();
  pointer.positionView = point.Position();
  pointer.timestamp = point.Timestamp() / 1000; // us -> ms
  pointer.pressure = props.Pressure();
  pointer.isBarrelButton = props.IsBarrelButtonPressed();
  pointer.shiftKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Shift));
  pointer.ctrlKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Control));
  pointer.altKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Menu));
}

std::optional<size_t> FabricTouchEventHandler::IndexOfPointerWithId(uint32_t pointerId) {
  for (size_t i = 0; i < m_pointers.size(); ++i) {
    if (m_pointers[i].pointerId == pointerId)
      return i;
  }

  return std::nullopt;
}

void FabricTouchEventHandler::UpdatePointersInViews(
    const winrt::PointerRoutedEventArgs &args,
    xaml::UIElement sourceElement,
    std::vector<int64_t> &&newViews) {
  if (auto uiManager = GetFabricUIManager(*m_context)) {
    int32_t pointerId = args.Pointer().PointerId();

    // m_pointers is tracking the pointers that are 'down', for moves we usually
    // don't have any pointers down and should reset the touchId back to zero
    if (m_pointers.size() == 0)
      m_touchId = 0;

    // Get the results of the last time we calculated the path
    auto it = m_pointersInViews.find(pointerId);
    TagSet *existingViews;
    if (it != m_pointersInViews.end()) {
      existingViews = &it->second;
    } else {
      existingViews = nullptr;
    }

    // Short-circuit if the hierarchy hasn't changed
    if ((existingViews == nullptr && newViews.size() == 0) ||
        (existingViews != nullptr && existingViews->orderedTags == newViews))
      return;

    // Prep to fire pointer events
    std::unordered_set<int64_t> newViewsSet(newViews.begin(), newViews.end());
    ReactPointer pointer;

    auto optPointerIndex = IndexOfPointerWithId(pointerId);
    if (optPointerIndex) {
      pointer = m_pointers[*optPointerIndex];
      UpdateReactPointer(pointer, args, sourceElement);
    } else {
      // newViews is empty when UpdatePointersInViews is called from outside
      // the root view, in this case use -1 for the JS event pointer target
      const auto tag = !newViews.empty() ? newViews.front() : InvalidTag;
      pointer = CreateReactPointer(args, tag, sourceElement);
    }

    // Walk through existingViews from innermost to outer, firing mouseLeave events if they are not in newViews
    if (existingViews) {
      for (int64_t existingTag : existingViews->orderedTags) {
        if (newViewsSet.count(existingTag)) {
          continue;
        }

        // TODO(T141828724): Enable onMouseLeave events for inline text
        const auto reactTag = static_cast<facebook::react::Tag>(existingTag);
        if (const auto view = std::static_pointer_cast<BaseComponentView const>(
                uiManager->GetViewRegistry().findComponentViewWithTag(reactTag))) {
          if (std::static_pointer_cast<facebook::react::ViewProps const>(view->props())
                  ->events[facebook::react::ViewEvents::Offset::MouseLeave]) {
            if (const auto emitter = view->GetEventEmitter(reactTag)) {
              emitter->onMouseLeave(TouchForPointer(pointer));
            }
          }
        }
      }
    }

    // Walk through newViews from outermost to inner, firing mouseEnter events if they are not in existingViews
    for (auto iter = newViews.rbegin(); iter != newViews.rend(); ++iter) {
      int64_t newTag = *iter;
      if (existingViews && existingViews->tags.count(newTag)) {
        continue;
      }

      // TODO(T141828724): Enable onMouseEnter events for inline text
      const auto reactTag = static_cast<facebook::react::Tag>(newTag);
      if (const auto view = std::static_pointer_cast<BaseComponentView const>(
              uiManager->GetViewRegistry().findComponentViewWithTag(reactTag))) {
        if (std::static_pointer_cast<facebook::react::ViewProps const>(view->props())
                ->events[facebook::react::ViewEvents::Offset::MouseEnter]) {
          if (const auto emitter = view->GetEventEmitter(reactTag)) {
            emitter->onMouseEnter(TouchForPointer(pointer));
          }
        }
      }
    }

    m_pointersInViews[pointerId] = {std::move(newViewsSet), std::move(newViews)};
  }
}

// defines button payload, follows https://developer.mozilla.org/docs/Web/API/MouseEvent/button
enum class MouseEventButtonKind { None = -1, Main = 0, Auxiliary = 1, Secondary = 2, Eraser = 5 };

facebook::react::SharedEventEmitter EventEmitterForElement(
    std::shared_ptr<FabricUIManager> &uimanager,
    facebook::react::Tag tag,
    std::vector<int64_t> tagsForBranch) noexcept {
  auto &registry = uimanager->GetViewRegistry();

  for (const auto ancestorTag : tagsForBranch) {
    if (const auto view = std::static_pointer_cast<BaseComponentView const>(
            registry.findComponentViewWithTag(static_cast<facebook::react::Tag>(ancestorTag)))) {
      if (const auto emitter = view->GetEventEmitter(tag)) {
        return emitter;
      }
    }
  }

  return nullptr;
}

facebook::react::Touch FabricTouchEventHandler::TouchForPointer(const ReactPointer &pointer) noexcept {
  MouseEventButtonKind button = MouseEventButtonKind::None;
  if (pointer.isLeftButton) {
    button = MouseEventButtonKind::Main;
  } else if (pointer.isMiddleButton) {
    button = MouseEventButtonKind::Auxiliary;
  } else if (pointer.isRightButton || pointer.isBarrelButton) {
    button = MouseEventButtonKind::Secondary;
  } else if (pointer.isEraser) {
    button = MouseEventButtonKind::Eraser;
  }

  facebook::react::Touch t;
  t.force = pointer.pressure;
  t.identifier = static_cast<int>(pointer.identifier);
  t.pagePoint.x = pointer.positionRoot.X;
  t.pagePoint.y = pointer.positionRoot.Y;
  t.offsetPoint.x = pointer.positionView.X;
  t.offsetPoint.y = pointer.positionView.Y;
  // TODO(T140472692): This should be relative to the rootview, not the XAML tree
  t.screenPoint.x = pointer.positionRoot.X;
  t.screenPoint.y = pointer.positionRoot.Y;
  t.target = static_cast<facebook::react::Tag>(pointer.target);
  t.timestamp = static_cast<facebook::react::Float>(pointer.timestamp);
  t.button = static_cast<int>(button);
  t.altKey = pointer.altKey;
  t.ctrlKey = pointer.ctrlKey;
  t.shiftKey = pointer.shiftKey;
  return t;
}

bool FabricTouchEventHandler::IsEndishEventType(TouchEventType eventType) noexcept {
  switch (eventType) {
    case TouchEventType::End:
    case TouchEventType::Cancel:
    case TouchEventType::CaptureLost:
      return true;
    default:
      return false;
  }
}

void FabricTouchEventHandler::DispatchTouchEvent(
    TouchEventType eventType,
    size_t pointerIndex,
    std::vector<int64_t> const &tagsForBranch) {
  winrt::Microsoft::ReactNative::JSValueArray changedIndices;
  changedIndices.push_back(pointerIndex);

  if (auto uiManager = GetFabricUIManager(*m_context)) {
    std::unordered_set<facebook::react::SharedTouchEventEmitter> uniqueEventEmitters = {};
    std::vector<facebook::react::SharedTouchEventEmitter> emittersForIndex;

    facebook::react::TouchEvent te;

    size_t index = 0;
    for (const auto &pointer : m_pointers) {
      bool isChangedPointer = pointerIndex == index++;

      if (!isChangedPointer || !IsEndishEventType(eventType)) {
        te.touches.insert(TouchForPointer(pointer));
      }

      if (isChangedPointer)
        te.changedTouches.insert(TouchForPointer(pointer));

      auto emitter = std::static_pointer_cast<facebook::react::TouchEventEmitter>(
          std::const_pointer_cast<facebook::react::EventEmitter>(
              EventEmitterForElement(uiManager, static_cast<facebook::react::Tag>(pointer.target), tagsForBranch)));
      emittersForIndex.push_back(emitter);
      if (emitter)
        uniqueEventEmitters.insert(emitter);
    }

    for (const auto emitter : uniqueEventEmitters) {
      te.targetTouches.clear();
      index = 0;
      for (const auto &pointer : m_pointers) {
        auto pointerEmitter = emittersForIndex[index++];
        if (emitter == pointerEmitter)
          te.targetTouches.insert(TouchForPointer(pointer));
      }

      switch (eventType) {
        case TouchEventType::Start:
          emitter->onTouchStart(te);
          break;
        case TouchEventType::Move:
          emitter->onTouchMove(te);
          break;
        case TouchEventType::End:
          emitter->onTouchEnd(te);
          break;
        case TouchEventType::Cancel:
        case TouchEventType::CaptureLost:
          emitter->onTouchCancel(te);
          break;
      }
    }
  }
}

bool FabricTouchEventHandler::DispatchBackEvent() {
  if (m_context->State() != Mso::React::ReactInstanceState::Loaded)
    return false;

  // TODO(T140425520): emit back press

  return true;
}

winrt::Microsoft::ReactNative::PointerEventKind FabricTouchEventHandler::GetPointerEventKind(
    TouchEventType eventType) noexcept {
  auto kind = winrt::Microsoft::ReactNative::PointerEventKind::None;
  switch (eventType) {
    case TouchEventType::Start:
      kind = winrt::Microsoft::ReactNative::PointerEventKind::Start;
      break;
    case TouchEventType::End:
      kind = winrt::Microsoft::ReactNative::PointerEventKind::End;
      break;
    case TouchEventType::Move:
      kind = winrt::Microsoft::ReactNative::PointerEventKind::Move;
      break;
    case TouchEventType::Cancel:
      kind = winrt::Microsoft::ReactNative::PointerEventKind::Cancel;
      break;
    case TouchEventType::CaptureLost:
      kind = winrt::Microsoft::ReactNative::PointerEventKind::CaptureLost;
      break;
    default:
      assert(false);
      break;
  }
  return kind;
}

bool FabricTouchEventHandler::PropagatePointerEventAndFindReactSourceBranch(
    const winrt::Microsoft::ReactNative::ReactPointerEventArgs &args,
    std::vector<int64_t> *pTagsForBranch,
    xaml::UIElement *pSourceElement) {
  assert(pTagsForBranch != nullptr);
  assert(pSourceElement != nullptr);

  if (const auto uiManager = GetFabricUIManager(*m_context)) {
    auto sourceElement = args.Args().OriginalSource().try_as<xaml::UIElement>();
    std::vector<int64_t> tagsForBranch;

    // Iterate over candidate views to find the source element and branch tags.
    // TODO(T140472320): add support for hovering over multiple "branches"
    // of React views. The current algorithm finds the first React view that
    // does not use "box-none" and walks that branch. A better fix would be to
    // find all branches via FindElementsInHostCoordinates.
    if (auto view = GetNearestReactViewAncestor(uiManager, sourceElement)) {
      // Do not consider views that have "box-none" set.
      // Only use FindElementsInHostCoordinates if absolutely necessary.
      // Use of "box-none" should be rare in react-native-windows, as you
      // can implement it by not setting a background on a View.
      if (std::static_pointer_cast<facebook::react::ViewProps const>(view->props())->pointerEvents ==
          facebook::react::PointerEventsMode::BoxNone) {
        // Get an iterator for all views at the current point.
        // VisualTreeHelper returns the list in z-index order.
        const auto rootReactElement = m_rootView.as<xaml::UIElement>();
        xaml::UIElement rootWindowElement =
            IsXamlIsland() ? rootReactElement.XamlRoot().Content() : winrt::Window::Current().Content();
        const auto rootPoint = args.Args().GetCurrentPoint(rootWindowElement).Position();
        const auto hitViews = winrt::VisualTreeHelper::FindElementsInHostCoordinates(rootPoint, rootReactElement);

        for (const auto hitView : hitViews) {
          if (view = GetNearestReactViewAncestor(uiManager, hitView)) {
            if (std::static_pointer_cast<facebook::react::ViewProps const>(view->props())->pointerEvents !=
                facebook::react::PointerEventsMode::BoxNone) {
              break;
            }
          }
        }
      }

      // Walk to root to find refined React target view and branch tags
      while (view) {
        const xaml::UIElement ancestorElement = view->Element();
        if (args.Target() == nullptr) {
          // Update the sourceElement, which is used for calculating relative pointer position
          sourceElement = ancestorElement;
          args.Target(ancestorElement);
        }

        // Allow the component view to refine the hit test target. For
        // example, Text may refine the specific span, or pointerEvents
        // prop values may update the target.
        const auto previousTarget = args.Target().try_as<XamlView>();
        view->OnPointerEvent(args);
        const auto target = args.Target().try_as<XamlView>();

        if (target != previousTarget) {
          tagsForBranch.clear();
          if (target) {
            // TODO(T140473055): we need to determine how to get all tags between the new and previous target
            tagsForBranch.push_back(GetTag(target));
          }

          if (auto targetElement = target.try_as<xaml::UIElement>()) {
            // Update the sourceElement, which is used for calculating relative pointer position
            sourceElement = targetElement;
          } else if (target) {
            // If the new Target is not a UIElement, use the current View's element instead
            sourceElement = ancestorElement;
          }
        }

        // Add the current ancestors tag to the branch
        const auto ancestorTag = GetTag(ancestorElement);
        if (target) {
          tagsForBranch.push_back(ancestorTag);
        }

        // Stop traversing when we get to the root target
        if (ancestorElement == m_xamlView) {
          break;
        }

        // A single component may compose multiple XAML elements each that
        // may or may not set the same tag on each element (e.g., View with
        // control wrapper). This prevents visiting the same view twice.
        auto tag = ancestorTag;
        xaml::DependencyObject parentElement = ancestorElement;
        while (tag == ancestorTag) {
          parentElement = winrt::VisualTreeHelper::GetParent(parentElement);
          tag = GetTag(parentElement);
        }

        view = GetNearestReactViewAncestor(uiManager, parentElement);
      }
    }

    if (args.Target() && sourceElement) {
      *pTagsForBranch = std::move(tagsForBranch);
      *pSourceElement = sourceElement;
      return true;
    }
  }

  // If the root view is not fully created, then the Tag property will never
  // be set. This can happen, e.g., when the red box error box is shown.
  return false;
}

} // namespace Microsoft::ReactNative
