// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>

#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <UI.Xaml.Controls.h>
#include <react/renderer/components/rnwcore/Props.h>

#pragma warning(push)
#pragma warning(disable : 4244 4305)
// #include <react/renderer/components/view/ViewProps.h>
#pragma warning(pop)

namespace Microsoft::ReactNative {

struct SliderComponentView : BaseComponentView {
  using Super = BaseComponentView;
  SliderComponentView();

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
  xaml::Controls::Slider m_element;
  xaml::Controls::Slider::ValueChanged_revoker m_valueChangedRevoker;
};

} // namespace Microsoft::ReactNative
