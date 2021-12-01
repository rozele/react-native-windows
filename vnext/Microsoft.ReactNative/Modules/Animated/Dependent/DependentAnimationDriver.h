// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>

namespace Microsoft::ReactNative {
typedef std::function<void(std::vector<folly::dynamic>)> Callback;

class DependentValueAnimatedNode;
class DependentNativeAnimatedNodeManager;
class DependentAnimationDriver {
 public:
  DependentAnimationDriver(
      int64_t id,
      int64_t animatedValueTag,
      const folly::dynamic &config,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  inline constexpr int64_t Id() {
    return m_id;
  };

  inline constexpr int64_t AnimatedValueTag() {
    return m_animatedValueTag;
  }

  inline Callback EndCallback() {
    return m_endCallback;
  }

  bool IsComplete() {
    return m_isComplete;
  }

  void RunAnimationStep(winrt::TimeSpan renderingTime);

 protected:
  bool m_isComplete{false};
  int64_t m_id{0};
  int64_t m_animatedValueTag{};
  Callback m_endCallback{};
  folly::dynamic m_config{};
  std::weak_ptr<DependentNativeAnimatedNodeManager> m_manager{};
  int64_t m_iterations{};
  int64_t m_iteration{0};
  double m_startFrameTimeMs{-1};

  static constexpr double s_frameDurationMs = 1000.0 / 60.0;

  virtual bool Update(double timeDeltaMs, bool restarting) = 0;

  DependentValueAnimatedNode *GetAnimatedValue();
};
} // namespace Microsoft::ReactNative
