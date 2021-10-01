// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "PaperLayoutAnimationController.h"
#include <Views/ViewManagerBase.h>

namespace react::uwp {

static bool IsNaNOrZero(double value) {
  return isnan(value) || std::abs(value) <= std::numeric_limits<double>::epsilon();
}

void PaperLayoutAnimationController::ConfigureLayoutAnimation(folly::dynamic &&config) noexcept {
  if (config == nullptr) {
    Reset();
    return;
  }

  // TODO(T102475060): Add support for mount and unmount animations
  const auto globalDuration = config["duration"].asInt();
  m_updateLayoutAnimation.ConfigureLayoutAnimation(config["update"], globalDuration);
  m_shouldAnimateLayout = true;
}

void PaperLayoutAnimationController::OnComplete(int64_t tag) noexcept {
  const auto iter = m_tagsToOperations.find(tag);
  if (iter != m_tagsToOperations.end()) {
    for (auto i = 0; i < iter->second.size(); ++i) {
      const auto &operation = iter->second.at(i);
      operation.storyboard.Stop();
      // TODO(T102475061): we skip layout operations for unfinished layout
      // changes. This may be a problem for components that expect `onLayout`
      // events to deterministically fire for each layout event.
      if (i == iter->second.size() - 1) {
        SetLayoutProps(tag, operation.left, operation.top, operation.width, operation.height);
      }
    }
  }
}

void PaperLayoutAnimationController::Reset() noexcept {
  m_shouldAnimateLayout = false;
  m_updateLayoutAnimation.Reset();
}

void PaperLayoutAnimationController::SetHost(facebook::react::INativeUIManagerHost *host) noexcept {
  m_host = host;
}

void PaperLayoutAnimationController::AnimateLayoutProps(
    ShadowNodeBase &node,
    float left,
    float top,
    float width,
    float height) {
#ifndef USE_WINUI3
  const auto element = node.GetView().as<xaml::FrameworkElement>();
  if (!IsNaNOrZero(element.Width()) && !IsNaNOrZero(element.Height())) {
    // Check if storyboard should be restarted
    const auto existingStoryboardEntry = m_tagsToOperations.find(node.m_tag);
    if (existingStoryboardEntry != m_tagsToOperations.end()) {
      // Pause any existing animations. Stopping these animations will reset
      // the animation progress. Retain the animation operation so we can stop
      // the latest animation completes.
      const auto existingOperations = existingStoryboardEntry->second;
      const auto lastIndex = existingStoryboardEntry->second.size();
      existingStoryboardEntry->second.at(lastIndex - 1).storyboard.Pause();
    }

    // Create callback to remove Storyboard, this callback is called even if the target is unmounted
    std::function<void()> onComplete = [weakSelf = weak_from_this(), tag = node.m_tag, left, top, width, height]() {
      if (const auto self = weakSelf.lock()) {
        self->OnComplete(tag);
      }
    };

    // Create a storyboard to animate layout, otherwise just set the layout
    if (const auto storyboard =
            m_updateLayoutAnimation.CreateAnimation(node, left, top, width, height, std::move(onComplete))) {
      storyboard.Begin();
      std::vector<AnimatedLayoutOperation> operations;
      AnimatedLayoutOperation operation{storyboard, left, top, width, height};
      if (existingStoryboardEntry != m_tagsToOperations.end()) {
        existingStoryboardEntry->second.push_back(operation);
      } else {
        m_tagsToOperations.insert({node.m_tag, {{operation}}});
      }
    } else {
      node.GetViewManager()->SetLayoutProps(node, node.GetView(), left, top, width, height);
    }
  }
#endif
}

void PaperLayoutAnimationController::SetLayoutProps(int64_t tag, float left, float top, float width, float height) {
  if (m_host) {
    if (const auto node = static_cast<ShadowNodeBase *>(m_host->FindShadowNodeForTag(tag))) {
      node->GetViewManager()->SetLayoutProps(*node, node->GetView(), left, top, width, height);
    }
  }
}

bool PaperLayoutAnimationController::ShouldAnimateLayout(XamlView const &view) {
#ifdef USE_WINUI3
  return false;
#else
  if (m_shouldAnimateLayout) {
    if (const auto frameworkElement = view.try_as<xaml::FrameworkElement>()) {
      return !!frameworkElement.Parent();
    }
  }

  return false;
#endif
}

} // namespace react::uwp
