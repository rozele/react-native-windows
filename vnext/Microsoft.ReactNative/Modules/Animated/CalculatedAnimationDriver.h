// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include <utility>
#include "AnimatedNode.h"
#include "AnimationDriver.h"

namespace Microsoft::ReactNative {
class CalculatedAnimationDriver : public AnimationDriver {
 public:
  using AnimationDriver::AnimationDriver;

  std::tuple<comp::CompositionAnimation, comp::CompositionScopedBatch> MakeAnimation(
      const folly::dynamic &config) override;

 protected:
  double m_startValue{0};
};
} // namespace Microsoft::ReactNative
