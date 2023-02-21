// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SwitchComponentView.h"

#include <Utils/ValueUtils.h>

#include <IReactContext.h>

#include <react/renderer/components/rnwcore/EventEmitters.h>

namespace Microsoft::ReactNative {

SwitchComponentView::SwitchComponentView() : m_element(xaml::Controls::ToggleSwitch()) {
  m_element.OnContent(nullptr);
  m_element.OffContent(nullptr);

  m_toggledRevoker = m_element.Toggled(winrt::auto_revoke, [this](auto sender, auto args) {
    const auto &props = *std::static_pointer_cast<const facebook::react::SwitchProps>(m_props);
    if (props.value != m_element.IsOn()) {
      if (m_eventEmitter) {
        auto emitter = std::static_pointer_cast<const facebook::react::SwitchEventEmitter>(m_eventEmitter);
        facebook::react::SwitchEventEmitter::OnChange onChangeArgs;
        onChangeArgs.value = m_element.IsOn();
        emitter->onChange(onChangeArgs);
      }
    }
  });

  static auto const defaultProps = std::make_shared<facebook::react::SwitchProps const>();
  m_props = defaultProps;
}

void SwitchComponentView::handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept {
  if (commandName == "setValue") {
    m_element.IsOn(arg[0].asBool());
  } else {
    Super::handleCommand(commandName, arg);
  }
}

void SwitchComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldSwitchProps = *std::static_pointer_cast<const facebook::react::SwitchProps>(m_props);
  const auto &newSwitchProps = *std::static_pointer_cast<const facebook::react::SwitchProps>(props);

  if (oldSwitchProps.value != newSwitchProps.value) {
    m_element.IsOn(newSwitchProps.value);
  }

  if (oldSwitchProps.disabled != newSwitchProps.disabled) {
    m_element.IsEnabled(!newSwitchProps.disabled);
  }

  // TODO tint colors

  Super::updateProps(props, oldProps);
}

void SwitchComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {}

void SwitchComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {}

void SwitchComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement SwitchComponentView::Element() const noexcept {
  return m_element;
}

} // namespace Microsoft::ReactNative
