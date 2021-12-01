// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimationDriver.h"

namespace Microsoft::ReactNative {
class DependentDecayAnimationDriver : public DependentAnimationDriver {
  using Super = DependentAnimationDriver;
 public:
  DependentDecayAnimationDriver(
      int64_t id,
      int64_t animatedValueTag,
      const folly::dynamic &config,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

 protected:
  bool Update(double timeDeltaMs, bool restarting) override;

 private:
  double m_velocity{0};
  double m_deceleration{0};
  double m_fromValue{0};
  double m_lastValue{0};
};
} // namespace Microsoft::ReactNative
