// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>

#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <Microsoft.ReactNative.Cxx/ReactContext.h>
#include <Views/Image/ReactImage.h>

#pragma warning(push)
#pragma warning(disable : 4244 4305)
#include <react/renderer/components/view/ViewProps.h>
#pragma warning(pop)

namespace Microsoft::ReactNative {

struct ImageComponentView : BaseComponentView {
  using Super = BaseComponentView;
  ImageComponentView(winrt::Microsoft::ReactNative::ReactContext const &reactContext);

  std::vector<facebook::react::ComponentDescriptorProvider> supplementalComponentDescriptorProviders() noexcept
      override;
  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;

  const xaml::FrameworkElement Element() const noexcept override;

 private:
  bool m_needsOnLoadStart{false};
  facebook::react::LayoutMetrics m_layoutMetrics;
  winrt::com_ptr<ReactImage> m_element{nullptr};
  winrt::Microsoft::ReactNative::ReactContext m_context;
  winrt::event_token m_onLoadEndToken;
};

} // namespace Microsoft::ReactNative
