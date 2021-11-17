// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "AnimationDriver.h"

namespace Microsoft::ReactNative {
struct ControlledAnimationState {
  int64_t iteration;
  double startTime;
};

class AnimationDriver;
class ControlledAnimationManager : public std::enable_shared_from_this<ControlledAnimationManager> {
 public:
  void StartAnimation(std::weak_ptr<AnimationDriver> weakAnimation);
  void StopAnimation(int64_t id);
 private:
  void OnRendering(winrt::IInspectable const &sender, winrt::IInspectable const &args);
  std::unordered_map<int64_t, std::weak_ptr<AnimationDriver>> m_activeAnimations;
  std::unordered_map<int64_t, ControlledAnimationState> m_animationState;
  xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;
};
} // namespace Microsoft::ReactNative
