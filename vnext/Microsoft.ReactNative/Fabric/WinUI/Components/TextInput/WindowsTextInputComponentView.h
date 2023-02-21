// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>
#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <Fabric/WinUI/FabricUIManagerModule.h>
#include <ReactContext.h>
#include "WindowsTextInputProps.h"
#include "WindowsTextInputShadowNode.h"

namespace Microsoft::ReactNative {

struct WindowsTextInputComponentView : BaseComponentView {
  using Super = BaseComponentView;
  WindowsTextInputComponentView(const winrt::Microsoft::ReactNative::ReactContext &context);

  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;
  void handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept override;

  virtual const xaml::FrameworkElement Element() const noexcept override;

 private:
  void registerEvents() noexcept;
  void registerPreviewKeyDown() noexcept;
  void SetText(winrt::hstring text) noexcept;
  facebook::react::AttributedString getAttributedString() const;
  void updatePropsTextBox(
      facebook::react::Props::Shared const &props,
      facebook::react::Props::Shared const &oldProps) noexcept;
  void updatePropsPasswordBox(
      facebook::react::Props::Shared const &props,
      facebook::react::Props::Shared const &oldProps) noexcept;
  void ReparentView(xaml::Controls::Control oldView);

  xaml::Controls::Control m_control{xaml::Controls::TextBox()};
  winrt::Microsoft::ReactNative::ReactContext m_context;

  xaml::Controls::TextBox::SelectionChanged_revoker m_SelectionChangedRevoker;
  xaml::Controls::TextBox::PreviewKeyDown_revoker m_controlPreviewKeyDownRevoker;
  xaml::Controls::TextBox::TextChanging_revoker m_textChangingRevoker;
  xaml::Controls::PasswordBox::PasswordChanging_revoker m_passwordBoxPasswordChangingRevoker;
  xaml::Controls::PasswordBox::PasswordChanged_revoker m_passwordBoxPasswordChangedRevoker;

  facebook::react::LayoutMetrics m_layoutMetrics;
  std::shared_ptr<facebook::react::WindowsTextInputShadowNode::ConcreteState const> m_state;
  int64_t m_mostRecentEventCount{0};
  int m_nativeEventCount{0};
  bool m_comingFromJS{false};
  bool m_comingFromState{false};
};

} // namespace Microsoft::ReactNative
