// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentAnimationDriver.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentAnimationDriver::DependentAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const folly::dynamic &config,
    const Callback &endCallback,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : m_id{id}, m_animatedValueTag{animatedValueTag}, m_endCallback{endCallback}, m_manager{manager} {
  m_iterations = [iterations = config.find("iterations"), end = config.items().end()]() {
    if (iterations != end) {
      return static_cast<int64_t>(iterations.dereference().second.asDouble());
    }
    return static_cast<int64_t>(1);
  }();

  m_isComplete = m_iterations == 0;
}

void DependentAnimationDriver::RunAnimationStep(winrt::TimeSpan renderingTime) {
  if (m_isComplete) {
    return;
  }

  const auto frameTimeMs = renderingTime.count() / 10000.0;
  auto restarting = false;
  if (m_startFrameTimeMs < 0) {
    m_startFrameTimeMs = frameTimeMs - s_frameDurationMs - 1e-6;
    restarting = true;
  }

  const auto timeDeltaMs = frameTimeMs - m_startFrameTimeMs;
  const auto isComplete = Update(timeDeltaMs, restarting);

  if (isComplete) {
    if (m_iterations == -1 || ++m_iteration < m_iterations) {
      m_startFrameTimeMs = -1;
    } else {
      m_isComplete = true;
    }
  }
}

DependentValueAnimatedNode *DependentAnimationDriver::GetAnimatedValue() {
  if (auto manager = m_manager.lock()) {
    return manager->GetValueAnimatedNode(m_animatedValueTag);
  }
  return nullptr;
}



} // namespace Microsoft::ReactNative
