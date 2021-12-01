// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentDecayAnimationDriver.h"
#include "DependentNativeAnimatedNodeManager.h"
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {

static constexpr auto FRAME_DURATION_MS = 1000.0 / 60.0;

DependentDecayAnimationDriver::DependentDecayAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const folly::dynamic& config,
    const Callback& endCallback,
    const std::shared_ptr<DependentNativeAnimatedNodeManager>& manager)
    : Super(id, animatedValueTag, config, endCallback, manager) {
  m_velocity = config["velocity"].asDouble();
  m_deceleration = config["deceleration"].asDouble();
}

bool DependentDecayAnimationDriver::Update(double timeDeltaMs, bool restarting) {
  if (const auto node = GetAnimatedValue()) {
    if (restarting) {
      const auto value = node->RawValue();
      if (m_fromValue == m_lastValue) {
        // First iteration, assign m_fromValue based on AnimatedValue
        m_fromValue = value;
      } else {
        // Not the first iteration, reset AnimatedValue based on m_fromValue
        node->RawValue(m_fromValue);
      }

      m_lastValue = value;
    }

    const auto value =
        m_fromValue + (m_velocity / (1 - m_deceleration)) * (1 - std::exp(-(1 - m_deceleration) * timeDeltaMs));
    if (std::abs(m_lastValue - value) >= 0.1) {
      m_lastValue = value;
      node->RawValue(value);
      return false;
    }
  }

  return true;
}
} // namespace Microsoft::ReactNative
