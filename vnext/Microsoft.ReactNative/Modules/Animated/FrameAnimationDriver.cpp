// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "FrameAnimationDriver.h"
#include "Utils/Helpers.h"

namespace Microsoft::ReactNative {
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

std::tuple<comp::CompositionPropertySet, comp::ScalarKeyFrameAnimation> FrameAnimationDriver::MakeKeyFrameAnimation() {
  const auto compositor = Microsoft::ReactNative::GetCompositor();
  const auto keyFramePS = compositor.CreatePropertySet();
  const auto animation = compositor.CreateScalarKeyFrameAnimation();

  // Frames contains 60 values per second of duration of the animation, convert
  // the size of frames to duration in ms.
  std::chrono::milliseconds duration(static_cast<int>(m_frames.size() * 1000.0 / 60.0));
  animation.Duration(duration);
  auto normalizedProgress = 0.0f;
  auto step = 1.0f / m_frames.size();
  for (auto frame : m_frames) {
    normalizedProgress = std::min(normalizedProgress += step, 1.0f);
    animation.InsertKeyFrame(normalizedProgress, static_cast<float>(frame));
  }

  keyFramePS.InsertScalar(s_frameValueName, static_cast<float>(m_frames[0]));

  if (m_iterations == -1) {
    animation.IterationBehavior(winrt::AnimationIterationBehavior::Forever);
  } else {
    animation.IterationCount(static_cast<int32_t>(m_iterations));
    animation.IterationBehavior(winrt::AnimationIterationBehavior::Count);
  }

  return std::make_tuple(keyFramePS, animation);
}

comp::ExpressionAnimation FrameAnimationDriver::MakeExpressionAnimation() {
  const auto compositor = Microsoft::ReactNative::GetCompositor();
  const auto animation = compositor.CreateExpressionAnimation();
  const auto expr = static_cast<winrt::hstring>(L"This.StartingValue + ") + s_framePropertySetName + L"." + s_frameValueName + L" * (" +
      winrt::to_hstring(ToValue()) + L" - This.StartingValue)";
  animation.Expression(expr);
  return animation;
}

double FrameAnimationDriver::ToValue() {
  return m_toValue;
}

} // namespace Microsoft::ReactNative
