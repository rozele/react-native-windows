// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentSpringAnimationDriver.h"
#include "DependentNativeAnimatedNodeManager.h"
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {

static constexpr auto MAX_DELTA_TIME_SEC = 0.064;
static constexpr auto SOLVER_TIMESTEP_SPEC = 0.001;

DependentSpringAnimationDriver::DependentSpringAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const folly::dynamic &config,
    const Callback &endCallback,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(id, animatedValueTag, config, endCallback, manager) {
  m_springStiffness = config.find("stiffness").dereference().second.asDouble();
  m_springDamping = config.find("damping").dereference().second.asDouble();
  m_springMass = config.find("mass").dereference().second.asDouble();
  m_initialVelocity = config.find("initialVelocity").dereference().second.asDouble();
  m_endValue = config.find("toValue").dereference().second.asDouble();
  m_restSpeedThreshold = config.find("restSpeedThreshold").dereference().second.asDouble();
  m_displacementFromRestThreshold =
      config.find("restDisplacementThreshold").dereference().second.asDouble();
  m_overshootClampingEnabled = config.find("overshootClamping").dereference().second.asBool();
}

bool DependentSpringAnimationDriver::Update(double timeDeltaMs, bool restarting) {
  if (const auto node = GetAnimatedValue()) {
    if (restarting) {
      const auto value = node->RawValue();
      if (!m_originalValue) {
        m_originalValue = value;
      } else {
        node->RawValue(m_originalValue.value());
      }

      m_startValue = m_currentState.position = value;
      m_timeAccumulator = 0.0;
    }

    Advance(timeDeltaMs / 1000.0);
    node->RawValue(m_currentState.position);
    return IsAtRest();
  }

  return true;
}

double DependentSpringAnimationDriver::GetDisplacementDistanceForState(PhysicsState state) {
  return std::abs(m_endValue - state.position);
}

bool DependentSpringAnimationDriver::IsAtRest() {
  return std::abs(m_currentState.velocity) <= m_restSpeedThreshold &&
      (GetDisplacementDistanceForState(m_currentState) <= m_displacementFromRestThreshold || m_springStiffness == 0);
}

bool DependentSpringAnimationDriver::IsOvershooting() {
  return m_springStiffness > 0 &&
      ((m_startValue < m_endValue && m_currentState.position > m_endValue) ||
       (m_startValue > m_endValue && m_currentState.position < m_endValue));
}

void DependentSpringAnimationDriver::Advance(double realDeltaTime) {
  if (IsAtRest()) {
    return;
  }

  // clamp the amount of realDeltaTime to avoid stuttering in the UI.
  // We should be able to catch up in a subsequent advance if necessary.
  auto adjustedDeltaTime = realDeltaTime;
  if (realDeltaTime > MAX_DELTA_TIME_SEC) {
    adjustedDeltaTime = MAX_DELTA_TIME_SEC;
  }

  m_timeAccumulator += adjustedDeltaTime;

  auto c = m_springDamping;
  auto m = m_springMass;
  auto k = m_springStiffness;
  auto v0 = -m_initialVelocity;

  auto zeta = c / (2.0 * std::sqrt(k * m));
  auto omega0 = std::sqrt(k / m);
  auto omega1 = omega0 * std::sqrt(1.0 - (zeta * zeta));
  auto x0 = m_endValue - m_startValue;

  double velocity;
  double position;
  const auto t = m_timeAccumulator;
  if (zeta < 1) {
    // Under damped
    double envelope = std::exp(-zeta * omega0 * t);
    position =
        m_endValue - envelope * ((v0 + zeta * omega0 * x0) / omega1 * std::sin(omega1 * t) + x0 * std::cos(omega1 * t));
    // This looks crazy -- it's actually just the derivative of the
    // oscillation function
    velocity = zeta * omega0 * envelope *
            (std::sin(omega1 * t) * (v0 + zeta * omega0 * x0) / omega1 + x0 * std::cos(omega1 * t)) -
        envelope * (std::cos(omega1 * t) * (v0 + zeta * omega0 * x0) - omega1 * x0 * std::sin(omega1 * t));
  } else {
    // Critically damped spring
    double envelope = std::exp(-omega0 * t);
    position = m_endValue - envelope * (x0 + (v0 + omega0 * x0) * t);
    velocity = envelope * (v0 * (t * omega0 - 1) + t * x0 * (omega0 * omega0));
  }

  m_currentState.position = position;
  m_currentState.velocity = velocity;

  // End the spring immediately if it is overshooting and overshoot clamping is enabled.
  // Also make sure that if the spring was considered within a resting threshold that it's now
  // snapped to its end value.
  if (IsAtRest() || (m_overshootClampingEnabled && IsOvershooting())) {
    // Don't call setCurrentValue because that forces a call to onSpringUpdate
    if (m_springStiffness > 0) {
      m_startValue = m_endValue;
      m_currentState.position = m_endValue;
    } else {
      m_endValue = m_currentState.position;
      m_startValue = m_endValue;
    }
    m_currentState.velocity = 0;
  }
}
} // namespace Microsoft::ReactNative
