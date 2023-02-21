// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <winrt/Microsoft.ReactNative.h>
#include "LegacyABIViewShadowNode.h"

namespace Microsoft::ReactNative {

struct LegacyABIViewComponentView : BaseComponentView {
  using Super = BaseComponentView;
  LegacyABIViewComponentView(winrt::Microsoft::ReactNative::IViewManager const &viewManager);
  ~LegacyABIViewComponentView();

  void OnPointerEvent(winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept override;
  void mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept override;
  void unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept override;
  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  void handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;
  void ReplaceChild(xaml::FrameworkElement const &oldView, xaml::FrameworkElement const &view) noexcept;

  virtual const xaml::FrameworkElement Element() const noexcept override;

 private:
  winrt::Microsoft::ReactNative::IViewManager m_viewManager;
  xaml::FrameworkElement m_element{nullptr};
  std::shared_ptr<facebook::react::LegacyABIViewShadowNode::ConcreteState const> m_state;
};

} // namespace Microsoft::ReactNative
