// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentFrameBasedAnimationDriver.h"
#include "DependentNativeAnimatedNodeManager.h"
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentFrameBasedAnimationDriver::DependentFrameBasedAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const folly::dynamic &config,
    const Callback& endCallback,
    const std::shared_ptr<DependentNativeAnimatedNodeManager>& manager)
    : Super(id, animatedValueTag, config, endCallback, manager) {
  for (const auto &frame : config.find("frames").dereference().second) {
    m_frames.push_back(frame.asDouble());
  }
  m_toValue = config.find("toValue").dereference().second.asDouble();
}

bool DependentFrameBasedAnimationDriver::Update(double timeDeltaMs, bool restarting) {
  if (const auto node = GetAnimatedValue()) {
    if (!m_fromValue) {
      m_fromValue = node->RawValue();
    }

    const auto fromValue = m_fromValue.value();
    const auto frameIndex = static_cast<size_t>(timeDeltaMs / s_frameDurationMs);
    assert(frameIndex >= 0);

    double nextValue;
    auto isComplete = false;
    if (frameIndex >= m_frames.size() - 1) {
      nextValue = m_toValue;
      isComplete = true;
    } else {
      nextValue = fromValue + m_frames[frameIndex] * (m_toValue - fromValue);
    }

    node->RawValue(nextValue);

    return isComplete;
  }

  return true;
}
} // namespace Microsoft::ReactNative
