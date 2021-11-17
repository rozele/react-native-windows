// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ControlledAnimationManager.h"
#include <UI.Xaml.Media.h>

namespace Microsoft::ReactNative {

void ControlledAnimationManager::StartAnimation(std::weak_ptr<AnimationDriver> weakAnimation) {
  // Nothing to do if the loop count is 0
  const auto animation = weakAnimation.lock();
  if (!animation || animation->m_iterations == 0)
    return;

  const auto isFirstAnimation = m_activeAnimations.size() == 0;
  m_activeAnimations[animation->Id()] = animation;
  if (isFirstAnimation) {
    m_renderingRevoker =
        xaml::Media::CompositionTarget::Rendering(winrt::auto_revoke, {this, &ControlledAnimationManager::OnRendering});
  }
}

void ControlledAnimationManager::StopAnimation(int64_t id) {
  m_activeAnimations.erase(id);
  m_animationState.erase(id);
  if (m_activeAnimations.size() == 0) {
    m_renderingRevoker.revoke();
  }
}

void ControlledAnimationManager::OnRendering(winrt::IInspectable const &sender, winrt::IInspectable const &args) {
  const auto renderingArgs = args.try_as<xaml::Media::RenderingEventArgs>();
  if (!renderingArgs)
    return;

  // The RenderingTime tick is in increments of 100 nanoseconds.
  // This converts the rendering time to milliseconds.
  const auto currentTime = renderingArgs.RenderingTime().count() / 10000.0;

  std::vector<int64_t> completedAnimations;
  for (auto entry : m_activeAnimations) {
    const auto animation = entry.second.lock();
    if (!animation)
      continue;

    const auto startTimeIter = m_animationState.find(entry.first);

    // Set the rendering start time if not set
    if (startTimeIter == m_animationState.end()) {
      m_animationState.insert({entry.first, {1, currentTime}});
    }

    // Compute the time delta
    auto &animationState = m_animationState[entry.first];
    const auto deltaTime = currentTime - animationState.startTime;

    // Get the value and completion status for the current time
    const auto [value, velocity] = animation->GetValueAndVelocityForTime(deltaTime);
    const auto isComplete = animation->IsAnimationDone(value, velocity);
    const auto valueNode = animation->GetAnimatedValue();

    // TODO: add support for non-Composition props
    valueNode->RawValue(value);

    // Restart loop or stop animation
    if (isComplete) {
      if (animation->m_iterations == -1 || animationState.iteration < animation->m_iterations) {
        animationState.startTime = currentTime;
        animationState.iteration++;
      } else {
        completedAnimations.push_back(animation->Id());
      }
    }
  }

  for (auto id : completedAnimations) {
    const auto entry = m_activeAnimations.find(id);
    if (entry == m_activeAnimations.end())
      continue;

    const auto animation = entry->second.lock();
    if (!animation)
      continue;

    animation->StopAnimation();
  }
}

} // namespace Microsoft::ReactNative
