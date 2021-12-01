// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimationDriver.h"

namespace Microsoft::ReactNative {
struct PhysicsState {
  double position{0.0};
  double velocity{0.0};
};

class DependentSpringAnimationDriver : public DependentAnimationDriver {
  using Super = DependentAnimationDriver;
 public:
  DependentSpringAnimationDriver(
      int64_t id,
      int64_t animatedValueTag,
      const folly::dynamic &config,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

 protected:
  bool Update(double timeDeltaMs, bool restarting) override;

 private:
  double GetDisplacementDistanceForState(PhysicsState state);
  bool IsAtRest();
  bool IsOvershooting();
  void Advance(double realTimeDelta);

  double m_springStiffness{0};
  double m_springDamping{0};
  double m_springMass{0};
  double m_initialVelocity{0};
  double m_endValue{0};
  double m_restSpeedThreshold{0};
  double m_displacementFromRestThreshold{0};
  bool m_overshootClampingEnabled{0};
  folly::dynamic m_dynamicToValues{};

  std::optional<double> m_originalValue;
  double m_timeAccumulator{0};
  double m_startValue{0};
  PhysicsState m_currentState{};
};
} // namespace Microsoft::ReactNative
