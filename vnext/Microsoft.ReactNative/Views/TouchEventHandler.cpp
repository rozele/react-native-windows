// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <pch.h>

#include <Views/ShadowNodeBase.h>
#include "TouchEventHandler.h"

#include <JSValueWriter.h>
#include <Modules/NativeUIManager.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <Utils/ValueUtils.h>
#include <Views/TextViewManager.h>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Devices.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>

#ifdef WINUI3_PREVIEW3
#include <winrt/Microsoft.UI.Input2.Experimental.h>
#endif

namespace react::uwp {

std::vector<int64_t> GetTagsForBranch(facebook::react::INativeUIManagerHost *host, int64_t tag, int64_t rootTag);

TouchEventHandler::TouchEventHandler(const std::weak_ptr<IReactInstance> &reactInstance)
    : m_xamlView(nullptr),
      m_rootView(nullptr),
      m_wkReactInstance(reactInstance),
      m_batchingEventEmitter{std::make_shared<BatchingEventEmitter>(reactInstance)} {}

TouchEventHandler::~TouchEventHandler() {
  RemoveTouchHandlers();
}

void TouchEventHandler::AddTouchHandlers(XamlView xamlView, bool findRoot, bool handledEventsToo) {
  auto uiElement(xamlView.as<xaml::UIElement>());
  if (uiElement == nullptr) {
    assert(false);
    return;
  }

  m_xamlView = xamlView;
  m_findRoot = findRoot;

  RemoveTouchHandlers();

  m_pressedHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerPressed});
  m_releasedHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerReleased});
  m_canceledHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerCanceled});
  m_captureLostHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerCaptureLost});
  m_exitedHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerExited});
  m_movedHandler = winrt::box_value(winrt::PointerEventHandler{this, &TouchEventHandler::OnPointerMoved});
  uiElement.AddHandler(xaml::UIElement::PointerPressedEvent(), m_pressedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerReleasedEvent(), m_releasedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerCanceledEvent(), m_canceledHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerCaptureLostEvent(), m_captureLostHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerExitedEvent(), m_exitedHandler, handledEventsToo);
  uiElement.AddHandler(xaml::UIElement::PointerMovedEvent(), m_movedHandler, handledEventsToo);
  m_subscribed = true;
}

void TouchEventHandler::RemoveTouchHandlers() {
  if (m_subscribed) {
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
    m_subscribed = false;
    m_rootView = nullptr;
    m_xamlView = nullptr;
  }
}

BatchingEventEmitter &TouchEventHandler::BatchingEmitter() noexcept {
  return *m_batchingEventEmitter;
}

void TouchEventHandler::OnPointerPressed(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  auto instance = m_wkReactInstance.lock();
  if (!instance || instance->IsInError())
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
  const auto argsImpl =
      winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(reactArgs);
  if (!PropagatePointerEventAndFindReactTarget(instance, reactArgs, &tagsForBranch, &sourceElement))
    return;

  // If this was caused by the user pressing the "back" hardware button, fire that event instead
  if (args.GetCurrentPoint(sourceElement).Properties().PointerUpdateKind() ==
      winrt::Windows::UI::Input::PointerUpdateKind::XButton1Pressed) {
    args.Handled(DispatchBackEvent());
    return;
  }

  if (!argsImpl->DefaultPrevented() &&
      (argsImpl->UncapturedAllowed() || m_xamlView.as<xaml::FrameworkElement>().CapturePointer(args.Pointer()))) {
    assert(!tagsForBranch.empty());
    const auto tag = tagsForBranch.front();

    // Pointer pressing updates the enter/leave state
    UpdatePointersInViews(instance, args, sourceElement, std::move(tagsForBranch));

    size_t pointerIndex = AddReactPointer(args, tag, sourceElement);

    // For now, when using the mouse we only want to send click events for the left button.
    // Finger and pen taps will also set isLeftButton.
    if (m_pointers[pointerIndex].isLeftButton) {
      DispatchTouchEvent(eventType, pointerIndex);
    }
  }
}

void TouchEventHandler::OnPointerReleased(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::End, args);
}

void TouchEventHandler::OnPointerCanceled(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::Cancel, args);
}

void TouchEventHandler::OnPointerCaptureLost(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  OnPointerConcluded(TouchEventType::CaptureLost, args);
}

void TouchEventHandler::OnPointerExited(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  auto instance = m_wkReactInstance.lock();
  if (!instance || instance->IsInError())
    return;

  std::vector<int64_t> tagsForBranch;
  UpdatePointersInViews(instance, args, nullptr, std::move(tagsForBranch));
}

void TouchEventHandler::OnPointerMoved(
    const winrt::IInspectable & /*sender*/,
    const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  auto instance = m_wkReactInstance.lock();
  if (!instance || instance->IsInError())
    return;

  // Only if the view has a Tag can we process this
  std::vector<int64_t> tagsForBranch;
  xaml::UIElement sourceElement(nullptr);
  const auto eventType = TouchEventType::Move;
  const auto kind = GetPointerEventKind(eventType);
  const auto reactArgs = winrt::make<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(kind, args);
  const auto argsImpl =
      winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(reactArgs);
  if (!PropagatePointerEventAndFindReactTarget(instance, reactArgs, &tagsForBranch, &sourceElement))
    return;

  const auto pointerId = args.Pointer().PointerId();
  const auto optPointerIndex = IndexOfPointerWithId(pointerId);
  if (optPointerIndex) {
    UpdateReactPointer(m_pointers[*optPointerIndex], args, sourceElement);
    if (!IsPointerCaptured(pointerId) && !argsImpl->UncapturedAllowed()) {
      OnPointerConcluded(TouchEventType::Cancel, args);
    } else if (!argsImpl->DefaultPrevented()) {
      DispatchTouchEvent(eventType, *optPointerIndex);
    }
  } else if (!argsImpl->DefaultPrevented()) {
    // Move with no buttons pressed
    UpdatePointersInViews(instance, args, sourceElement, std::move(tagsForBranch));
  }
}

void TouchEventHandler::OnPointerConcluded(TouchEventType eventType, const winrt::PointerRoutedEventArgs &args) {
  // Short circuit all of this if we are in an error state
  auto instance = m_wkReactInstance.lock();
  if (!instance || instance->IsInError())
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
  const auto argsImpl =
      winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(reactArgs);
  if (PropagatePointerEventAndFindReactTarget(instance, reactArgs, &tagsForBranch, &sourceElement))
    UpdateReactPointer(m_pointers[*optPointerIndex], args, sourceElement);

  if (eventType == TouchEventType::CaptureLost && argsImpl->UncapturedAllowed()) {
    return;
  } else {
    const auto modifiedEventType = argsImpl->DefaultPrevented()
        ? TouchEventType::Cancel
        : reactArgs.Kind() == winrt::Microsoft::ReactNative::PointerEventKind::End ? TouchEventType::End : eventType;
    DispatchTouchEvent(modifiedEventType, *optPointerIndex);
  }

  m_pointers.erase(cbegin(m_pointers) + *optPointerIndex);
  if (m_pointers.size() == 0)
    m_touchId = 0;

  const auto wasCaptured = IsPointerCaptured(args.Pointer().PointerId());

  m_xamlView.as<xaml::FrameworkElement>().ReleasePointerCapture(args.Pointer());

  // Updates the enter/leave state when pointer was previously captured
  if (wasCaptured)
    UpdatePointersInViews(instance, args, sourceElement, std::move(tagsForBranch));
}

size_t TouchEventHandler::AddReactPointer(
    const winrt::PointerRoutedEventArgs &args,
    int64_t tag,
    xaml::UIElement sourceElement) {
  ReactPointer pointer = CreateReactPointer(args, tag, sourceElement);
  m_pointers.emplace_back(std::move(pointer));
  return m_pointers.size() - 1;
}

TouchEventHandler::ReactPointer TouchEventHandler::CreateReactPointer(
    const winrt::PointerRoutedEventArgs &args,
    int64_t tag,
    xaml::UIElement sourceElement) {
  auto point = args.GetCurrentPoint(sourceElement);
  auto props = point.Properties();

  ReactPointer pointer;
  pointer.target = tag;
  pointer.identifier = m_touchId++;
  pointer.pointerId = point.PointerId();
#ifndef WINUI3_PREVIEW3
  pointer.deviceType = point.PointerDevice().PointerDeviceType();
#else
  pointer.deviceType = point.PointerDeviceType();
#endif
  pointer.isLeftButton = props.IsLeftButtonPressed();
  pointer.isRightButton = props.IsRightButtonPressed();
  pointer.isMiddleButton = props.IsMiddleButtonPressed();
  pointer.isHorizontalScrollWheel = props.IsHorizontalMouseWheel();
#ifndef WINUI3_PREVIEW3
  pointer.isEraser = props.IsEraser();
#endif

  UpdateReactPointer(pointer, args, sourceElement);

  return pointer;
}

void TouchEventHandler::UpdateReactPointer(
    ReactPointer &pointer,
    const winrt::PointerRoutedEventArgs &args,
    xaml::UIElement sourceElement) {
  const auto parentTag = GetTag(m_xamlView);
  auto rootView = m_xamlView;
  if (m_findRoot) {
    if (!m_rootView) {
      if (const auto host = GetNativeUIManagerHost(m_wkReactInstance)) {
        const auto rootNode = static_cast<ShadowNodeBase *>(host->FindParentRootShadowNode(parentTag));
        if (rootNode) {
          m_rootView = rootNode->GetView();
          rootView = m_rootView;
        }
      }
    } else {
      rootView = m_rootView;
    }
  }

  auto rootPoint = args.GetCurrentPoint(rootView.as<xaml::FrameworkElement>());
  auto point = args.GetCurrentPoint(sourceElement);
  auto props = point.Properties();
  auto keyModifiers = static_cast<uint32_t>(args.KeyModifiers());

  pointer.positionRoot = rootPoint.Position();
  pointer.positionView = point.Position();
  pointer.timestamp = point.Timestamp() / 1000; // us -> ms
#ifndef WINUI3_PREVIEW3
  pointer.pressure = props.Pressure();
#endif
  pointer.isBarrelButton = props.IsBarrelButtonPressed();
  pointer.shiftKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Shift));
  pointer.ctrlKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Control));
  pointer.altKey = 0 != (keyModifiers & static_cast<uint32_t>(winrt::Windows::System::VirtualKeyModifiers::Menu));
}

std::optional<size_t> TouchEventHandler::IndexOfPointerWithId(uint32_t pointerId) {
  for (size_t i = 0; i < m_pointers.size(); ++i) {
    if (m_pointers[i].pointerId == pointerId)
      return i;
  }

  return std::nullopt;
}

void TouchEventHandler::UpdatePointersInViews(
    std::shared_ptr<IReactInstance> instance,
    const winrt::PointerRoutedEventArgs &args,
    xaml::UIElement sourceElement,
    std::vector<int64_t> &&newViews) {
  const auto host = GetNativeUIManagerHost(instance);
  if (host == nullptr)
    return;

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
    // tagsForBranch is empty when UpdatePointersInViews is called from outside
    // the root view, in this case use -1 for the JS event pointer target
    const auto tag = !newViews.empty() ? newViews.front() : -1;
    pointer = CreateReactPointer(args, tag, sourceElement);
  }

  // Walk through existingViews from innermost to outer, firing mouseLeave events if they are not in newViews
  if (existingViews) {
    for (int64_t existingTag : existingViews->orderedTags) {
      if (newViewsSet.count(existingTag)) {
        continue;
      }

      ShadowNodeBase *node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(existingTag));
      if (node != nullptr && node->m_onMouseLeaveRegistered)
        BatchingEmitter().DispatchEvent(
            existingTag,
            L"topMouseLeave",
            winrt::Microsoft::ReactNative::MakeJSValueWriter(GetPointerJson(pointer, existingTag)));
    }
  }

  // Walk through newViews from outermost to inner, firing mouseEnter events if they are not in existingViews
  for (auto iter = newViews.rbegin(); iter != newViews.rend(); ++iter) {
    int64_t newTag = *iter;
    if (existingViews && existingViews->tags.count(newTag)) {
      continue;
    }

    ShadowNodeBase *node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(newTag));
    if (node != nullptr && node->m_onMouseEnterRegistered)
      BatchingEmitter().DispatchEvent(
          newTag, L"topMouseEnter", winrt::Microsoft::ReactNative::MakeJSValueWriter(GetPointerJson(pointer, newTag)));
  }

  m_pointersInViews[pointerId] = {std::move(newViewsSet), std::move(newViews)};
}

winrt::Microsoft::ReactNative::JSValue TouchEventHandler::GetPointerJson(const ReactPointer &pointer, int64_t target) {
  return winrt::Microsoft::ReactNative::JSValueObject{
      {"target", target},
      {"identifier", pointer.identifier},
      {"pageX", pointer.positionRoot.X},
      {"pageY", pointer.positionRoot.Y},
      {"locationX", pointer.positionView.X},
      {"locationY", pointer.positionView.Y},
      {"timestamp", pointer.timestamp},
      {
          "pointerType",
          GetPointerDeviceTypeName(pointer.deviceType),
      },
      {"force", pointer.pressure},
      {"isLeftButton", pointer.isLeftButton},
      {"isRightButton", pointer.isRightButton},
      {"isMiddleButton", pointer.isMiddleButton},
      {"isBarrelButtonPressed", pointer.isBarrelButton},
      {"isHorizontalScrollWheel", pointer.isHorizontalScrollWheel},
      {"isEraser", pointer.isEraser},
      {"shiftKey", pointer.shiftKey},
      {"ctrlKey", pointer.ctrlKey},
      {"altKey", pointer.altKey}};
}

void TouchEventHandler::DispatchTouchEvent(TouchEventType eventType, size_t pointerIndex) {
  winrt::Microsoft::ReactNative::JSValueArray changedIndices;
  changedIndices.push_back(pointerIndex);

  winrt::Microsoft::ReactNative::JSValueArray touches;
  for (const auto &pointer : m_pointers) {
    touches.push_back(GetPointerJson(pointer, pointer.target));
  }

  // Package up parameters and invoke the JS event emitter
  const wchar_t *eventName = GetTouchEventTypeName(eventType);
  if (eventName == nullptr)
    return;

  const auto paramsWriter = MakeJSValueArgWriter(eventName, std::move(touches), std::move(changedIndices));
  if (eventType == TouchEventType::Move || eventType == TouchEventType::PointerMove) {
    BatchingEmitter().EmitCoalescingJSEvent(
        L"RCTEventEmitter", L"receiveTouches", std::move(eventName), m_pointers[pointerIndex].pointerId, paramsWriter);
  } else {
    BatchingEmitter().EmitJSEvent(L"RCTEventEmitter", L"receiveTouches", paramsWriter);
  }
}

bool TouchEventHandler::DispatchBackEvent() {
  auto instance = m_wkReactInstance.lock();
  if (instance != nullptr && !instance->IsInError()) {
    BatchingEmitter().EmitJSEvent(
        L"RCTDeviceEventEmitter", L"emit", winrt::Microsoft::ReactNative::MakeJSValueArgWriter(L"hardwardBackPress"));
    return true;
  }

  return false;
}

const char *TouchEventHandler::GetPointerDeviceTypeName(
    winrt::Windows::Devices::Input::PointerDeviceType deviceType) noexcept {
  const char *deviceTypeName = "unknown";
  switch (deviceType) {
    case winrt::Windows::Devices::Input::PointerDeviceType::Mouse:
      deviceTypeName = "mouse";
      break;
    case winrt::Windows::Devices::Input::PointerDeviceType::Pen:
      deviceTypeName = "pen";
      break;
    case winrt::Windows::Devices::Input::PointerDeviceType::Touch:
      deviceTypeName = "touch";
      break;
    default:
      break;
  }
  return deviceTypeName;
}

winrt::Microsoft::ReactNative::PointerEventKind TouchEventHandler::GetPointerEventKind(
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

const wchar_t *TouchEventHandler::GetTouchEventTypeName(TouchEventType eventType) noexcept {
  const wchar_t *eventName = nullptr;
  switch (eventType) {
    case TouchEventType::Start:
      eventName = L"topTouchStart";
      break;
    case TouchEventType::End:
      eventName = L"topTouchEnd";
      break;
    case TouchEventType::Move:
      eventName = L"topTouchMove";
      break;
    case TouchEventType::Cancel:
    case TouchEventType::CaptureLost:
      eventName = L"topTouchCancel";
      break;
    default:
      assert(false);
      break;
  }
  return eventName;
}

bool TouchEventHandler::PropagatePointerEventAndFindReactTarget(
    std::shared_ptr<IReactInstance> instance,
    const winrt::Microsoft::ReactNative::ReactPointerEventArgs &args,
    std::vector<int64_t> *pTagsForBranch,
    xaml::UIElement *pSourceElement) {
  assert(pTagsForBranch != nullptr);
  assert(pSourceElement != nullptr);

  const auto host = GetNativeUIManagerHost(instance);
  if (host == nullptr)
    return false;

  xaml::UIElement sourceElement = args.Args().OriginalSource().try_as<xaml::UIElement>();
  ShadowNodeBase *node = nullptr;
  std::vector<int64_t> tagsForBranch;

  // Find the "deepest" React element that triggered the input event
  while (sourceElement) {
    node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(GetTag(sourceElement)));
    if (node) {
      args.Target(sourceElement);
      break;
    } else {
      sourceElement = winrt::VisualTreeHelper::GetParent(sourceElement).try_as<xaml::UIElement>();
    }
  }

  // Walk to root to find refined React target view
  const auto argsImpl = winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactPointerEventArgs>(args);
  while (node) {
    if (args.Target() == nullptr) {
      args.Target(node->GetView());
    }

    const auto previousTarget = args.Target();

    node->GetViewManager()->OnPointerEvent(node, args);

    const auto target = args.Target().try_as<XamlView>();
    if (args.Target() != previousTarget) {
      tagsForBranch.clear();
      if (target) {
        // We assume that if a ViewManager is going to change the target, it
        // can only update the target to one of its descendants.
        const auto tagsToCurrentTarget = GetTagsForBranch(host, GetTag(target), node->m_tag);
        for (auto tag : tagsToCurrentTarget) {
          tagsForBranch.push_back(tag);
        }
      }
    }

    if (target) {
      tagsForBranch.push_back(node->m_tag);
    }

    // Stop traversing when we get to the root target
    if (node->GetView() == m_xamlView) {
      break;
    }

    node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(node->m_parent));
  }

  if (args.Target() != nullptr) {
    sourceElement = args.Target().try_as<xaml::UIElement>();

    // Find the first parent UIElement of the React target for pointer positioning
    if (!sourceElement) {
      node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(tagsForBranch.front()));
    }

    while (!sourceElement && node) {
      node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(node->m_parent));
      sourceElement = node->GetView().try_as<xaml::UIElement>();
    }

    if (sourceElement) {
      // If a TextBlock was the UIElement event source, perform a more accurate hit test,
      // searching for the tag of the nested Run/Span XAML elements that the user actually clicked.
      // This is to support nested <Text> elements in React.
      // Nested React <Text> elements get translated into nested XAML <Span> elements,
      // while the content of the <Text> becomes a list of XAML <Run> elements.
      // However, we should report the Text element as the target, not the contexts of the text.
      const auto tag = GetTag(args.Target().as<XamlView>());
      if (tag > 0) {
        if (const auto textBlock = sourceElement.try_as<xaml::Controls::TextBlock>()) {
          // No need to hit test if TextBlock does not use Inlines
          if (textBlock.Inlines().Size() > 0) {
            if (const auto host = GetNativeUIManagerHost(m_wkReactInstance)) {
              const auto node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(tag));
              const auto pointerPos = args.Args().GetCurrentPoint(textBlock).RawPosition();
              const auto finerTag = TextViewManager::GetReactTagAtPoint(node, pointerPos);
              const auto tagsToCurrentTarget = GetTagsForBranch(host, finerTag, tag);
              // Insert the nested tags in reverse order
              auto iter = tagsToCurrentTarget.rbegin();
              while (iter != tagsToCurrentTarget.rend()) {
                tagsForBranch.insert(tagsForBranch.begin(), *iter);
                iter++;
              }
            }
          }
        }
      }

      *pTagsForBranch = std::move(tagsForBranch);
      *pSourceElement = sourceElement;
      return true;
    }
  }

  // If the root view is not fully created, then the Tag property will never
  // be set. This can happen, e.g., when the red box error box is shown.
  return false;
}

bool TouchEventHandler::IsPointerCaptured(uint32_t pointerId) {
  const auto frameworkElement = m_xamlView.as<xaml::FrameworkElement>();
  if (frameworkElement.PointerCaptures()) {
    for (auto pointer : frameworkElement.PointerCaptures()) {
      if (pointer.PointerId() == pointerId) {
        return true;
      }
    }
  }

  return false;
}

//
// Retreives the path of nodes from an element to the root.
// The order of the returned list is from child to parent.
//
std::vector<int64_t> GetTagsForBranch(facebook::react::INativeUIManagerHost *host, int64_t tag, int64_t rootTag) {
  std::vector<int64_t> tags;

  auto *shadowNode = host->FindShadowNodeForTag(tag);
  while (shadowNode != nullptr && tag != -1) {
    if (tag == rootTag) {
      break;
    }

    tags.push_back(tag);
    tag = shadowNode->m_parent;
    shadowNode = host->FindShadowNodeForTag(tag);
  }

  return tags;
}

} // namespace react::uwp
