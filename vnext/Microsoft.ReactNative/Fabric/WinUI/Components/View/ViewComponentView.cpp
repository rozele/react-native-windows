// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ViewComponentView.h"

#include <UI.Composition.h>
#include <UI.Xaml.Controls.h>
#include <Utils/ResourceBrushUtils.h>
#include <Utils/ValueUtils.h>
#include <Views/FrameworkElementTransferProperties.h>
#include <XamlView.h>

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

  if ((!oldProps || newViewProps.transform != oldViewProps.transform && newViewProps.transform.operations.size() > 0) &&
      !propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN.count("transform")) {
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

  m_props = props;
}

void BaseComponentView::updateEventEmitter(facebook::react::EventEmitter::Shared const &eventEmitter) noexcept {
  m_eventEmitter = std::static_pointer_cast<facebook::react::ViewEventEmitter const>(eventEmitter);
}

const facebook::react::SharedViewEventEmitter &BaseComponentView::GetEventEmitter(
    facebook::react::Tag tag) const noexcept {
  return m_eventEmitter;
}

void BaseComponentView::handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept {
  assert(false); // Unhandled command
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

std::vector<facebook::react::ComponentDescriptorProvider>
ViewComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void ViewComponentView::mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  m_panel.Children().InsertAt(index, static_cast<const BaseComponentView &>(childComponentView).Element());
}

void ViewComponentView::unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept {
  m_panel.Children().RemoveAt(index);
}

void ViewComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ViewProps>(props);

  if (oldViewProps.backgroundColor != newViewProps.backgroundColor) {
    auto color = *newViewProps.backgroundColor;

    if (newViewProps.backgroundColor) {
      m_panel.ViewBackground(newViewProps.backgroundColor.AsWindowsBrush());
    } else {
      m_panel.ClearValue(winrt::Microsoft::ReactNative::ViewPanel::ViewBackgroundProperty());
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

  Super::updateProps(props, oldProps);
}

bool ViewComponentView::isFocusable() const noexcept {
  const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
  return props.focusable;
}

bool ViewComponentView::isAccessible() const noexcept {
  return false; // HasDynamicAutomationProperties(view);
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

  winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_panel, layoutMetrics.frame.origin.x);
  winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_panel, layoutMetrics.frame.origin.y);
}

void ViewComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  if (m_needsBorderUpdate) {
    const auto &props = *std::static_pointer_cast<const facebook::react::ViewProps>(m_props);
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

  auto oldElement = Element();
  auto parent = oldElement.Parent();

  bool needsControl = isFocusable();
  if ((bool)m_control != needsControl) {
    if (needsControl && !m_control) {
      m_control = winrt::Microsoft::ReactNative::ViewControl{};
      m_control.UseSystemFocusVisuals(m_enableFocusRing);
    }

    auto newElement = Element();

    // -- Transfer properties to new element
    SetTag(newElement, GetTag(oldElement));
    TransferFrameworkElementProperties(oldElement, newElement);

    // -- if the root element changes, we need to modify our parents child
    if (parent && oldElement != newElement) {
      // RefreshProperties(); - Transfer EnableFocusRing,TabIndex,IsFocusable
      auto parentPanel = parent.try_as<xaml::Controls::Panel>();
      if (parentPanel) {
        uint32_t index;
        auto found = parentPanel.Children().IndexOf(oldElement, index);
        assert(found);
        parentPanel.Children().SetAt(index, newElement);
      } else {
        assert(false); // TODO handle border/control parent
      }
    }

    if (m_control) {
      m_control.Content(m_panel);
    }
  }

  if (m_control) {
    m_control.IsTabStop(isFocusable());
  }

  if (m_control) {
    m_control.Width(m_layoutMetrics.frame.size.width);
    m_control.Height(m_layoutMetrics.frame.size.height);
  }

  m_panel.Width(m_layoutMetrics.frame.size.width);
  m_panel.Height(m_layoutMetrics.frame.size.height);

  if (m_control) {
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_control, m_layoutMetrics.frame.origin.x);
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_control, m_layoutMetrics.frame.origin.y);
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_panel, 0);
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_panel, 0);
  } else {
    winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_panel, m_layoutMetrics.frame.origin.x);
    winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_panel, m_layoutMetrics.frame.origin.y);
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
