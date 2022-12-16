// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>
#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <react/renderer/components/text/ParagraphProps.h>

namespace Microsoft::ReactNative {

struct ParagraphComponentView : BaseComponentView {
  using Super = BaseComponentView;
  ParagraphComponentView();

  std::vector<facebook::react::ComponentDescriptorProvider> supplementalComponentDescriptorProviders() noexcept
      override;
  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  const facebook::react::SharedViewEventEmitter &GetEventEmitter(facebook::react::Tag tag) const noexcept override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;

  const xaml::FrameworkElement Element() const noexcept override;
  void OnPointerEvent(winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept override;

 private:
  facebook::react::LayoutMetrics m_layoutMetrics;
  xaml::Controls::TextBlock m_element;
  std::unordered_map<facebook::react::Tag, facebook::react::SharedViewEventEmitter> m_fragmentEventEmitters{};
};

} // namespace Microsoft::ReactNative
