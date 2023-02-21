// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "LegacyABIViewComponentView.h"
#include <DynamicReader.h>
#include <Fabric/WinUI/FabricUIManagerModule.h>
#include "LegacyABIViewProps.h"

namespace Microsoft::ReactNative {

LegacyABIViewComponentView::LegacyABIViewComponentView(winrt::Microsoft::ReactNative::IViewManager const &viewManager)
    : m_viewManager{viewManager} {
  static auto const defaultProps = std::make_shared<facebook::react::LegacyABIViewProps const>();
  m_props = defaultProps;
  m_element = viewManager.CreateView();
}

LegacyABIViewComponentView::~LegacyABIViewComponentView() {
  if (const auto viewManagerWithDropViewInstance =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithDropViewInstance>()) {
    viewManagerWithDropViewInstance.OnDropViewInstance(m_element);
  }
}

void LegacyABIViewComponentView::OnPointerEvent(
    winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept {
  if (const auto viewManagerWithPointerEvents =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithPointerEvents>()) {
    viewManagerWithPointerEvents.OnPointerEvent(m_element, args);
  }

  Super::OnPointerEvent(args);
}

void LegacyABIViewComponentView::mountChildComponentView(
    const IComponentView &childComponentView,
    uint32_t index) noexcept {
  if (const auto viewManagerWithChildren =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithChildren>()) {
    viewManagerWithChildren.AddView(
        m_element, static_cast<const BaseComponentView &>(childComponentView).Element(), index);
  } else {
    Super::mountChildComponentView(childComponentView, index);
  }
}

void LegacyABIViewComponentView::unmountChildComponentView(
    const IComponentView &childComponentView,
    uint32_t index) noexcept {
  if (const auto viewManagerWithChildren =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithChildren>()) {
    viewManagerWithChildren.RemoveChildAt(m_element, index);
  } else {
    Super::unmountChildComponentView(childComponentView, index);
  }
}

void LegacyABIViewComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  if (const auto viewManagerWithNativeProperties =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithNativeProperties>()) {
    const auto &newProps = *std::static_pointer_cast<const facebook::react::LegacyABIViewProps>(props);
    winrt::Microsoft::ReactNative::IJSValueReader jsValueReader =
        winrt::make<winrt::Microsoft::ReactNative::DynamicReader>(newProps.otherProps);
    viewManagerWithNativeProperties.UpdateProperties(m_element, jsValueReader);
  }

  Super::updateProps(props, oldProps);
}

void LegacyABIViewComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {
  m_state = std::static_pointer_cast<facebook::react::LegacyABIViewShadowNode::ConcreteState const>(state);
}

void LegacyABIViewComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  if (const auto viewManagerWithOnLayout =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithOnLayout>()) {
    viewManagerWithOnLayout.OnLayout(
        m_element,
        layoutMetrics.frame.origin.x,
        layoutMetrics.frame.origin.y,
        layoutMetrics.frame.size.width,
        layoutMetrics.frame.size.height);
  }

  Super::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
}

void LegacyABIViewComponentView::handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept {
  if (const auto viewManagerWithCommands =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithCommands>()) {
    uint32_t unusedIndex;
    const auto convertedCommandName = winrt::to_hstring(commandName);
    if (viewManagerWithCommands.Commands().IndexOf(convertedCommandName, unusedIndex)) {
      const auto argReader = winrt::make<winrt::Microsoft::ReactNative::DynamicReader>(arg);
      viewManagerWithCommands.DispatchCommand(m_element, convertedCommandName, argReader);
      return;
    }
  }

  Super::handleCommand(commandName, arg);
}

void LegacyABIViewComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {}

void LegacyABIViewComponentView::prepareForRecycle() noexcept {}

void LegacyABIViewComponentView::ReplaceChild(
    xaml::FrameworkElement const &oldView,
    xaml::FrameworkElement const &view) noexcept {
  if (const auto viewManagerWithChildren =
          m_viewManager.try_as<winrt::Microsoft::ReactNative::IViewManagerWithChildren>()) {
    viewManagerWithChildren.ReplaceChild(m_element, oldView, view);
  } else {
    Super::ReplaceChild(oldView, view);
  }
}

const xaml::FrameworkElement LegacyABIViewComponentView::Element() const noexcept {
  return m_element;
}

} // namespace Microsoft::ReactNative
