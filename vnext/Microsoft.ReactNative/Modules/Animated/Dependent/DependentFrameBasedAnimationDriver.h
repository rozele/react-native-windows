// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimationDriver.h"

namespace Microsoft::ReactNative {
typedef std::function<void(std::vector<folly::dynamic>)> Callback;

class DependentFrameBasedAnimationDriver : public DependentAnimationDriver {
  using Super = DependentAnimationDriver;
 public:
  DependentFrameBasedAnimationDriver(
      int64_t id,
      int64_t animatedValueTag,
      const folly::dynamic &config,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

 protected:
  bool Update(double timeDeltaMs, bool restarting) override;

 private:
  std::vector<double> m_frames{};
  double m_toValue{0};
  std::optional<double> m_fromValue{};
};
} // namespace Microsoft::ReactNative
