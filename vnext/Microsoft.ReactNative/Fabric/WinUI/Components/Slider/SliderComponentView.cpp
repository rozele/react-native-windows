// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SliderComponentView.h"

#include <UI.Xaml.Controls.Primitives.h>
#include <Utils/ValueUtils.h>

#include <IReactContext.h>

#include <react/renderer/components/rnwcore/EventEmitters.h>

namespace Microsoft::ReactNative {

SliderComponentView::SliderComponentView() : m_element(xaml::Controls::Slider()) {
  static auto const defaultProps = std::make_shared<facebook::react::SliderProps const>();
  m_props = defaultProps;

  m_valueChangedRevoker = m_element.ValueChanged(winrt::auto_revoke, [this](auto sender, auto args) {
    const auto &props = *std::static_pointer_cast<const facebook::react::SliderProps>(m_props);
    if (props.value != m_element.Value()) {
      if (m_eventEmitter) {
        auto emitter = std::static_pointer_cast<const facebook::react::SliderEventEmitter>(m_eventEmitter);
        facebook::react::SliderEventEmitter::OnValueChange onValueChangeArgs;
        onValueChangeArgs.value = m_element.Value();
        emitter->onValueChange(onValueChangeArgs);
      }
    }
  });
}

void SliderComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldSliderProps = *std::static_pointer_cast<const facebook::react::SliderProps>(m_props);
  const auto &newSliderProps = *std::static_pointer_cast<const facebook::react::SliderProps>(props);

  if (oldSliderProps.value != newSliderProps.value) {
    m_element.Value(newSliderProps.value);
  }

  if (oldSliderProps.maximumValue != newSliderProps.maximumValue) {
    m_element.Maximum(newSliderProps.maximumValue);
  }

  if (oldSliderProps.minimumValue != newSliderProps.minimumValue) {
    m_element.Minimum(newSliderProps.minimumValue);
  }

  if (oldSliderProps.disabled != newSliderProps.disabled) {
    m_element.IsEnabled(!newSliderProps.disabled);
  }

  // TODO tint colors

  Super::updateProps(props, oldProps);
}

void SliderComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {}

void SliderComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {}

void SliderComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement SliderComponentView::Element() const noexcept {
  return m_element;
}

} // namespace Microsoft::ReactNative
