// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Fabric/WinUI/ComponentView.h>
#include <Fabric/WinUI/FabricKeyboardEventHandler.h>
#include <Views/ExpressionAnimationStore.h>
#include <react/renderer/components/view/ViewEventEmitter.h>
#include <react/renderer/components/view/ViewProps.h>

namespace Microsoft::ReactNative {

struct BaseComponentView : IComponentView {
  virtual const xaml::FrameworkElement Element() const noexcept = 0;
  comp::CompositionPropertySet EnsureCenterPointPropertySet() noexcept;
  virtual void OnPointerEvent(winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept;
  virtual void mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept;
  virtual void unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept;
  virtual void updateProps(
      facebook::react::Props::Shared const &props,
      facebook::react::Props::Shared const &oldProps) noexcept override;
  void updateEventEmitter(facebook::react::EventEmitter::Shared const &eventEmitter) noexcept override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  virtual const facebook::react::SharedViewEventEmitter &GetEventEmitter(facebook::react::Tag tag) const noexcept;
  void handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept override;
  facebook::react::Props::Shared props() const noexcept override;
  virtual void ReplaceChild(xaml::FrameworkElement const &oldView, xaml::FrameworkElement const &view) noexcept;

 protected:
  facebook::react::SharedViewEventEmitter m_eventEmitter;
  facebook::react::Props::Shared m_props;

 private:
  static std::shared_ptr<ExpressionAnimationStore> EnsureExpressionAnimationStore() noexcept;
  static void StartTransformAnimation(
      xaml::UIElement const &element,
      comp::CompositionPropertySet const &propertySet) noexcept;

  void ApplyTransformMatrix(winrt::Windows::Foundation::Numerics::float4x4 matrix) noexcept;
  void UpdateCenterPointPropertySet() noexcept;
  void EnsureKeyboardEventHandler() noexcept;

  comp::CompositionPropertySet m_centerPointPropertySet{nullptr};

  std::unique_ptr<FabricHandledKeyboardEventHandler> m_keyboardEventHandler{nullptr};
};

struct ViewComponentView : BaseComponentView {
  using Super = BaseComponentView;
  ViewComponentView();

  void mountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept override;
  void unmountChildComponentView(const IComponentView &childComponentView, uint32_t index) noexcept override;
  void updateProps(facebook::react::Props::Shared const &props, facebook::react::Props::Shared const &oldProps) noexcept
      override;
  void updateState(facebook::react::State::Shared const &state, facebook::react::State::Shared const &oldState) noexcept
      override;
  void updateLayoutMetrics(
      facebook::react::LayoutMetrics const &layoutMetrics,
      facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept override;
  void finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept override;
  void prepareForRecycle() noexcept override;
  void ReplaceChild(xaml::FrameworkElement const &oldView, xaml::FrameworkElement const &view) noexcept override;

  virtual const xaml::FrameworkElement Element() const noexcept;

 private:
  bool isFocusable() const noexcept;
  bool isAccessible() const noexcept;
  bool isHoverable() const noexcept;

  bool m_needsBorderUpdate{false};
  facebook::react::LayoutMetrics m_layoutMetrics;
  winrt::Microsoft::ReactNative::ViewControl m_control{nullptr};
  winrt::Microsoft::ReactNative::ViewPanel m_panel;
};

} // namespace Microsoft::ReactNative
