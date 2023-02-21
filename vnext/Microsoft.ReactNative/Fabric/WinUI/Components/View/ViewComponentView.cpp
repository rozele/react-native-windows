// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ViewComponentView.h"

#include <Fabric/WinUI/FabricUIManagerModule.h>
#include <ReactRootView.h>
#include <UI.Composition.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <Utils/ResourceBrushUtils.h>
#include <Utils/ValueUtils.h>
#include <Views/FrameworkElementTransferProperties.h>
#include <Views/Impl/ScrollViewViewChanger.h>
#include <XamlView.h>
#include "Unicode.h"

namespace Microsoft::ReactNative {

void BaseComponentView::mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  assert(false);
}

void BaseComponentView::unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  assert(false);
}

void BaseComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(props);

  const auto hasInitialTransform = !oldProps && newViewProps.transform != facebook::react::Transform::Identity();
  const auto hasUpdatedTransform = oldProps && newViewProps.transform != oldViewProps.transform;
  const auto hasAnimatedTransform = propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN.count("transform");
  if ((hasInitialTransform || hasUpdatedTransform) && !hasAnimatedTransform) {
    winrt::Windows::Foundation::Numerics::float4x4 matrix;
    matrix.m11 = newViewProps.transform.matrix[0];
    matrix.m12 = newViewProps.transform.matrix[1];
    matrix.m13 = newViewProps.transform.matrix[2];
    matrix.m14 = newViewProps.transform.matrix[3];
    matrix.m21 = newViewProps.transform.matrix[4];
    matrix.m22 = newViewProps.transform.matrix[5];
    matrix.m23 = newViewProps.transform.matrix[6];
    matrix.m24 = newViewProps.transform.matrix[7];
    matrix.m31 = newViewProps.transform.matrix[8];
    matrix.m32 = newViewProps.transform.matrix[9];
    matrix.m33 = newViewProps.transform.matrix[10];
    matrix.m34 = newViewProps.transform.matrix[11];
    matrix.m41 = newViewProps.transform.matrix[12];
    matrix.m42 = newViewProps.transform.matrix[13];
    matrix.m43 = newViewProps.transform.matrix[14];
    matrix.m44 = newViewProps.transform.matrix[15];
    const auto element = Element();
    if (!element.IsLoaded()) {
      element.Loaded([this, matrix](auto &&...) { ApplyTransformMatrix(matrix); });
    } else {
      ApplyTransformMatrix(matrix);
    }
  }

  if ((!oldProps || oldViewProps.opacity != newViewProps.opacity) &&
      !propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN.count("opacity")) {
    Element().Opacity(newViewProps.opacity);
  }

  if (!oldProps || oldViewProps.pointerEvents != newViewProps.pointerEvents) {
    if (newViewProps.pointerEvents == facebook::react::PointerEventsMode::None) {
      Element().IsHitTestVisible(false);
    } else {
      Element().ClearValue(xaml::UIElement::IsHitTestVisibleProperty());
    }
  }

  if (newViewProps.keyDownEvents.size() > 0) {
    EnsureKeyboardEventHandler();
    m_keyboardEventHandler->UpdateHandledKeyboardEvents(FabricKeyEventType::Down, newViewProps.keyDownEvents);
  } else if (oldProps && oldViewProps.keyDownEvents.size() > 0) {
    m_keyboardEventHandler->UpdateHandledKeyboardEvents(FabricKeyEventType::Down, newViewProps.keyDownEvents);
  }

  if (newViewProps.keyUpEvents.size() > 0) {
    EnsureKeyboardEventHandler();
    m_keyboardEventHandler->UpdateHandledKeyboardEvents(FabricKeyEventType::Up, newViewProps.keyUpEvents);
  } else if (oldProps && oldViewProps.keyUpEvents.size() > 0) {
    m_keyboardEventHandler->UpdateHandledKeyboardEvents(FabricKeyEventType::Up, newViewProps.keyUpEvents);
  }

  if (newViewProps.tooltip) {
    xaml::Controls::ToolTipService::SetToolTip(
        Element(), winrt::box_value(Microsoft::Common::Unicode::Utf8ToUtf16(newViewProps.tooltip.value())));
  } else if (oldProps && oldViewProps.tooltip) {
    Element().ClearValue(xaml::Controls::ToolTipService::ToolTipProperty());
  }

  if (newViewProps.overflowAnchor == "none") {
    Element().SetValue(ScrollViewViewChanger::CanBeScrollAnchorProperty(), winrt::box_value(false));
  } else if (oldProps && oldViewProps.overflowAnchor == "none") {
    Element().ClearValue(ScrollViewViewChanger::CanBeScrollAnchorProperty());
  }

  m_props = props;
}

void BaseComponentView::updateEventEmitter(facebook::react::EventEmitter::Shared const &eventEmitter) noexcept {
  m_eventEmitter = std::static_pointer_cast<facebook::react::ViewEventEmitter const>(eventEmitter);
}

void BaseComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  // Set Position & Size Properties
  const auto element = Element();
  if (layoutMetrics.frame.origin.x != oldLayoutMetrics.frame.origin.x) {
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(element, layoutMetrics.frame.origin.x);
  }
  if (layoutMetrics.frame.origin.y != oldLayoutMetrics.frame.origin.y) {
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(element, layoutMetrics.frame.origin.y);
  }
  if (layoutMetrics.frame.size.width != oldLayoutMetrics.frame.size.width) {
    element.Width(layoutMetrics.frame.size.width);
  }
  if (layoutMetrics.frame.size.height != oldLayoutMetrics.frame.size.height) {
    element.Height(layoutMetrics.frame.size.height);
  }

  const auto &viewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  if (viewProps.yogaStyle.overflow() == YGOverflowHidden) {
    xaml::Media::RectangleGeometry clipGeometry;
    clipGeometry.Rect(winrt::Rect(0, 0, layoutMetrics.frame.size.width, layoutMetrics.frame.size.height));
    element.Clip(clipGeometry);
  } else {
    element.ClearValue(xaml::UIElement::ClipProperty());
  }
}

const facebook::react::SharedViewEventEmitter &BaseComponentView::GetEventEmitter(
    facebook::react::Tag tag) const noexcept {
  return m_eventEmitter;
}

void BaseComponentView::handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept {
  if (commandName == "focus") {
    xaml::Input::FocusManager::TryFocusAsync(Element(), xaml::FocusState::Programmatic);
  } else if (commandName == "blur") {
    const auto element = Element();
    if (element == xaml::Input::FocusManager::GetFocusedElement(element.XamlRoot())) {
      if (const auto rootView = FabricUIManager::RootViewForView(this)) {
        rootView.as<winrt::Microsoft::ReactNative::implementation::ReactRootView>()->blur(element);
      }
    }
  }
}

void BaseComponentView::ReplaceChild(
    xaml::FrameworkElement const &oldView,
    xaml::FrameworkElement const &view) noexcept {
  assert(false);
}

facebook::react::Props::Shared BaseComponentView::props() const noexcept {
  return m_props;
}

void BaseComponentView::OnPointerEvent(
    winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept {
  const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  switch (props.pointerEvents) {
    case facebook::react::PointerEventsMode::None: {
      args.Target(nullptr);
      break;
    }
    case facebook::react::PointerEventsMode::BoxNone: {
      if (args.Target() == Element()) {
        args.Target(nullptr);
      }
      break;
    }
    case facebook::react::PointerEventsMode::BoxOnly: {
      args.Target(Element());
      break;
    }
  }
}
void BaseComponentView::ApplyTransformMatrix(winrt::Windows::Foundation::Numerics::float4x4 matrix) noexcept {
  // Get our PropertySet from the ShadowNode and insert the TransformMatrix as
  // the "transform" property
  auto propertySet = EnsureCenterPointPropertySet();
  propertySet.InsertMatrix4x4(L"transform", matrix);

  // Start the overall animation to multiply everything together
  StartTransformAnimation(Element(), propertySet);
}

comp::CompositionPropertySet BaseComponentView::EnsureCenterPointPropertySet() noexcept {
  if (m_centerPointPropertySet == nullptr) {
    auto compositor = GetCompositor(Element());
    m_centerPointPropertySet = compositor.CreatePropertySet();
    UpdateCenterPointPropertySet();
    m_centerPointPropertySet.InsertMatrix4x4(L"transform", winrt::Windows::Foundation::Numerics::float4x4::identity());
    m_centerPointPropertySet.InsertVector3(L"translation", {0, 0, 0});
  }

  return m_centerPointPropertySet;
}

void BaseComponentView::UpdateCenterPointPropertySet() noexcept {
  if (m_centerPointPropertySet != nullptr) {
    // First build up an ExpressionAnimation to compute the "center" property,
    // like so: The input to the expression is UIElement.ActualSize/2, output is
    // a vector3 with [cx, cy, 0].
    auto view = Element();
    assert(view != nullptr);
    m_centerPointPropertySet.InsertVector3(L"center", {0, 0, 0});

    auto centeringAnimation = EnsureExpressionAnimationStore()->GetElementCenterPointExpression(GetCompositor(view));
    centeringAnimation.SetExpressionReferenceParameter(L"uielement", view);
    m_centerPointPropertySet.StartAnimation(L"center", centeringAnimation);

    // Now insert the "transform" property with an initial value of identity.
    // The caller will handle populating this with the appropriate value (either
    // a static or animated value).
    winrt::Windows::Foundation::Numerics::float4x4 unused;

    // Take care not to stomp over any transform value we currently have set, as
    // we will use this value in the scenario where a View changed its backing
    // XAML element, here we will just transfer existing value to a new backing
    // XAML element.
    if (m_centerPointPropertySet.TryGetMatrix4x4(L"transform", unused) == comp::CompositionGetValueStatus::NotFound) {
      m_centerPointPropertySet.InsertMatrix4x4(
          L"transform", winrt::Windows::Foundation::Numerics::float4x4::identity());
    }
  }
}

void BaseComponentView::EnsureKeyboardEventHandler() noexcept {
  if (!m_keyboardEventHandler) {
    m_keyboardEventHandler = std::make_unique<FabricHandledKeyboardEventHandler>();
    m_keyboardEventHandler->hook(Element());
  }
}

/*static*/ std::shared_ptr<ExpressionAnimationStore> BaseComponentView::EnsureExpressionAnimationStore() noexcept {
  static std::shared_ptr<ExpressionAnimationStore> expressions;
  if (!expressions) {
    expressions = std::make_shared<ExpressionAnimationStore>();
  }

  return expressions;
}

/*static*/ void BaseComponentView::StartTransformAnimation(
    xaml::UIElement const &element,
    comp::CompositionPropertySet const &propertySet) noexcept {
  auto expression = EnsureExpressionAnimationStore()->GetTransformCenteringExpression(GetCompositor(element));
  expression.SetReferenceParameter(L"PS", propertySet);
  expression.Target(L"TransformMatrix");
  element.StartAnimation(expression);
}

ViewComponentView::ViewComponentView() {
  static auto const defaultProps = std::make_shared<facebook::react::ViewProps const>();
  m_props = defaultProps;
}

void ViewComponentView::mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  m_panel.Children().InsertAt(index, static_cast<const BaseComponentView &>(childComponentView).Element());
}

void ViewComponentView::unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  m_panel.Children().RemoveAt(index);
}

void ViewComponentView::ReplaceChild(
    xaml::FrameworkElement const &oldView,
    xaml::FrameworkElement const &view) noexcept {
  uint32_t index;
  if (m_panel.Children().IndexOf(oldView.as<xaml::UIElement>(), index)) {
    m_panel.Children().RemoveAt(index);
    m_panel.Children().InsertAt(index, view.as<xaml::UIElement>());
  }
}

void ViewComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(props);

  if (oldViewProps.backgroundColor != newViewProps.backgroundColor) {
    if (newViewProps.backgroundColor) {
      m_panel.Background(newViewProps.backgroundColor.AsWindowsBrush());
    } else {
      m_panel.ClearValue(xaml::Controls::Panel::BackgroundProperty());
    }
  }

  if (oldViewProps.borderColors != newViewProps.borderColors) {
    if (newViewProps.borderColors.all) {
      m_panel.BorderBrush(newViewProps.borderColors.all->AsWindowsBrush());
    } else {
      m_panel.ClearValue(xaml::Controls::Grid::BorderBrushProperty());
    }
  }

  if (oldViewProps.borderStyles != newViewProps.borderStyles || oldViewProps.borderRadii != newViewProps.borderRadii) {
    m_needsBorderUpdate = true;
  }

  if (oldViewProps.enableFocusRing != newViewProps.enableFocusRing && m_control) {
    m_control.UseSystemFocusVisuals(newViewProps.enableFocusRing);
  }

  Super::updateProps(props, oldProps);
}

bool ViewComponentView::isFocusable() const noexcept {
  const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  return props.focusable;
}

bool ViewComponentView::isAccessible() const noexcept {
  return false; // HasDynamicAutomationProperties(view);
}

bool ViewComponentView::isHoverable() const noexcept {
  const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  return props.windowsEvents[facebook::react::WindowsViewEvents::Offset::MouseEnter] ||
      props.windowsEvents[facebook::react::WindowsViewEvents::Offset::MouseLeave];
}

void ViewComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {}

void ViewComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  // Set Position & Size Properties
  m_needsBorderUpdate = true;
  m_layoutMetrics = layoutMetrics;

  Super::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
  if (m_control) {
    m_panel.Width(layoutMetrics.frame.size.width);
    m_panel.Height(layoutMetrics.frame.size.height);
  }
}

void ViewComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  if (m_needsBorderUpdate) {
    auto const borderMetrics = props.resolveBorderMetrics(m_layoutMetrics);
    m_panel.BorderThickness(xaml::ThicknessHelper::FromLengths(
        borderMetrics.borderWidths.left,
        borderMetrics.borderWidths.top,
        borderMetrics.borderWidths.right,
        borderMetrics.borderWidths.bottom));

    xaml::CornerRadius cornerRadius;
    cornerRadius.BottomLeft = borderMetrics.borderRadii.bottomLeft;
    cornerRadius.BottomRight = borderMetrics.borderRadii.bottomRight;
    cornerRadius.TopLeft = borderMetrics.borderRadii.topLeft;
    cornerRadius.TopRight = borderMetrics.borderRadii.topRight;

    if (borderMetrics.borderWidths != facebook::react::BorderWidths{} &&
        m_panel.ReadLocalValue(xaml::Controls::Grid::BorderBrushProperty()) == xaml::DependencyProperty::UnsetValue()) {
      m_panel.BorderBrush(DefaultBrushStore::Instance().GetDefaultBorderBrush());
    }

    m_panel.CornerRadius(cornerRadius);

    m_needsBorderUpdate = false;
  }

  if (m_panel.ReadLocalValue(xaml::Controls::Panel::BackgroundProperty()) == xaml::DependencyProperty::UnsetValue() &&
      isHoverable()) {
    m_panel.Background(facebook::react::clearColor().AsWindowsBrush());
  }

  if (isFocusable() && !m_control) {
    m_control = winrt::Microsoft::ReactNative::ViewControl{};

    const auto weakControl = winrt::make_weak(m_control);
    std::weak_ptr<const facebook::react::ViewEventEmitter> weakEmitter = m_eventEmitter;
    m_control.GotFocus([weakControl, weakEmitter](auto &&, auto &&args) {
      if (const auto emitter = weakEmitter.lock()) {
        if (const auto control = weakControl.get()) {
          if (args.OriginalSource() == control) {
            emitter->onFocus();
          }
        }
      }
    });

    m_control.LostFocus([weakControl, weakEmitter](auto &&, auto &&args) {
      if (const auto emitter = weakEmitter.lock()) {
        if (const auto control = weakControl.get()) {
          if (args.OriginalSource() == control) {
            emitter->onBlur();
          }
        }
      }
    });

    m_control.UseSystemFocusVisuals(props.enableFocusRing);

    // -- Transfer properties to new element
    SetTag(m_control, GetTag(m_panel));
    TransferFrameworkElementProperties(m_panel, m_control);

    // -- if the root element changes, we need to modify our parents child
    auto parent = m_panel.Parent();
    if (parent) {
      // RefreshProperties(); - Transfer EnableFocusRing,TabIndex,IsFocusable
      auto parentPanel = parent.try_as<xaml::Controls::Panel>();
      if (parentPanel) {
        uint32_t index;
        auto found = parentPanel.Children().IndexOf(m_panel, index);
        assert(found);
        parentPanel.Children().SetAt(index, m_control);
      } else {
        assert(false); // TODO handle border/control parent
      }
    }

    m_control.Content(m_panel);
    m_control.Width(m_layoutMetrics.frame.size.width);
    m_control.Height(m_layoutMetrics.frame.size.height);
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_control, m_layoutMetrics.frame.origin.x);
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_control, m_layoutMetrics.frame.origin.y);
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_panel, 0);
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_panel, 0);
  }

  if (m_control) {
    m_control.IsTabStop(isFocusable());
  }
}

void ViewComponentView::prepareForRecycle() noexcept {}

// View is implemented with up to three elements, which get nested
// 1) ViewControl  - if focusable
// 2) Outer Border - if rounded corners or other clipping
// 3) ViewPanel
const xaml::FrameworkElement ViewComponentView::Element() const noexcept {
  return m_control != nullptr ? m_control.as<xaml::FrameworkElement>() : m_panel;
}

} // namespace Microsoft::ReactNative
