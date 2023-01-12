// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>
#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <Fabric/WinUI/FabricTouchEventHandler.h>
#include <Microsoft.ReactNative.Cxx/ReactContext.h>
#include <react/renderer/components/text/ParagraphProps.h>

namespace Microsoft::ReactNative {

struct ParagraphComponentView : BaseComponentView {
  using Super = BaseComponentView;
  ParagraphComponentView(winrt::Microsoft::ReactNative::ReactContext const &reactContext);

  std::vector<facebook::react::ComponentDescriptorProvider> supplementalComponentDescriptorProviders() noexcept
      override;
  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  const facebook::react::SharedViewEventEmitter &GetEventEmitter(facebook::react::Tag tag) const noexcept override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;

  const xaml::FrameworkElement Element() const noexcept override;
  void OnPointerEvent(winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept override;

 private:
  void ToggleTouchEvents(bool selectable);

  xaml::Controls::TextBlock m_element;
  std::unordered_map<facebook::react::Tag, facebook::react::SharedViewEventEmitter> m_fragmentEventEmitters{};

  winrt::Microsoft::ReactNative::ReactContext m_context;
  std::shared_ptr<FabricTouchEventHandler> m_touchEventHandler;
  std::shared_ptr<bool> m_selectionChanged = std::make_shared<bool>(false);
  winrt::event_revoker<xaml::Controls::ITextBlock> m_selectionChangedRevoker;
};

} // namespace Microsoft::ReactNative
