// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "FrameAnimationDriver.h"
#include "Utils/Helpers.h"

namespace Microsoft::ReactNative {

const double FRAME_TIME_MILLISECONDS = 1000.0 / 60.0;

FrameAnimationDriver::FrameAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const Callback &endCallback,
    const folly::dynamic &config,
    const std::shared_ptr<NativeAnimatedNodeManager> &manager)
    : AnimationDriver(id, animatedValueTag, endCallback, config, manager) {
  for (const auto &frame : config.find("frames").dereference().second) {
    m_frames.push_back(frame.asDouble());
  }
  m_toValue = config.find("toValue").dereference().second.asDouble();
}

std::tuple<comp::CompositionAnimation, comp::CompositionScopedBatch> FrameAnimationDriver::MakeAnimation(
    const folly::dynamic & /*config*/) {
  const auto [scopedBatch, animation] = []() {
    const auto compositor = Microsoft::ReactNative::GetCompositor();
    return std::make_tuple(
        compositor.CreateScopedBatch(
            IsRS5OrHigher() ? comp::CompositionBatchTypes::AllAnimations : comp::CompositionBatchTypes::Animation),
        compositor.CreateScalarKeyFrameAnimation());
  }();

  // Frames contains 60 values per second of duration of the animation, convert
  // the size of frames to duration in ms.
  std::chrono::milliseconds duration(static_cast<int>(m_frames.size() * FRAME_TIME_MILLISECONDS));
  animation.Duration(duration);

  auto normalizedProgress = 0.0f;
  auto step = 1.0f / m_frames.size();
  auto fromValue = GetAnimatedValue()->RawValue();
  for (auto frame : m_frames) {
    normalizedProgress = std::min(normalizedProgress += step, 1.0f);
    animation.InsertKeyFrame(normalizedProgress, static_cast<float>(frame * (m_toValue - fromValue)));
  }

  if (m_iterations == -1) {
    animation.IterationBehavior(winrt::AnimationIterationBehavior::Forever);
  } else {
    animation.IterationCount(static_cast<int32_t>(m_iterations));
    animation.IterationBehavior(winrt::AnimationIterationBehavior::Count);
  }

  return std::make_tuple(animation, scopedBatch);
}

double FrameAnimationDriver::ToValue() {
  return m_toValue;
}

std::tuple<float, double> FrameAnimationDriver::GetValueAndVelocityForTime(double time) {
  assert(time >= 0);
  const auto frameIndex = static_cast<int>(time / FRAME_TIME_MILLISECONDS);
  if (frameIndex >= m_frames.size()) {
    return std::make_tuple(static_cast<float>(m_toValue), 0.0);
  }

  const auto fromValue = GetAnimatedValue()->RawValue();
  const auto value = fromValue + m_frames[frameIndex] * (m_toValue - fromValue);
  return std::make_tuple(static_cast<float>(value), 0.0);
}

bool FrameAnimationDriver::IsAnimationDone(double currentValue, double currentVelocity) {
  // Use float epsilon since the value is converted from double to float
  return std::abs(currentValue - m_toValue) <= std::numeric_limits<float>().epsilon();
}

} // namespace Microsoft::ReactNative
