// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ActivityIndicatorComponentView.h"

#include <UI.Xaml.Controls.h>
#include <Utils/ValueUtils.h>

#include <react/components/rnwcore/Props.h>

namespace Microsoft::ReactNative {

// TODO: m_element needed?
ActivityIndicatorComponentView::ActivityIndicatorComponentView() : m_element(xaml::Controls::ProgressRing()) {
  static auto const defaultProps = std::make_shared<facebook::react::ActivityIndicatorViewProps const>();
  m_props = defaultProps;
}

std::vector<facebook::react::ComponentDescriptorProvider>
ActivityIndicatorComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void ActivityIndicatorComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldActivityProps = *std::static_pointer_cast<const facebook::react::ActivityIndicatorViewProps>(m_props);
  const auto &newActivityProps = *std::static_pointer_cast<const facebook::react::ActivityIndicatorViewProps>(props);

  if (oldActivityProps.animating != newActivityProps.animating) {
    m_element.IsActive(newActivityProps.animating);
  }

  if (oldActivityProps.color != newActivityProps.color) {
    m_element.Foreground(newActivityProps.color.AsWindowsBrush());
  }

  Super::updateProps(props, oldProps);
}

void ActivityIndicatorComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {}
void ActivityIndicatorComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {}
void ActivityIndicatorComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement ActivityIndicatorComponentView::Element() const noexcept {
  return m_element;
}

} // namespace Microsoft::ReactNative
