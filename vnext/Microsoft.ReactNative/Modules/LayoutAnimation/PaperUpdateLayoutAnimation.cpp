// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "PaperUpdateLayoutAnimation.h"
#include <UI.Xaml.Media.h>
#include <Utils/ValueUtils.h>
#include <Views/ViewPanel.h>

namespace react::uwp {

#ifndef USE_WINUI3
xaml::Media::Animation::Storyboard PaperUpdateLayoutAnimation::CreateAnimation(
    ShadowNodeBase &node,
    float left,
    float top,
    float width,
    float height,
    std::function<void()> &&onComplete) {
  // TODO(T102475062): animate resize layout updates
  const auto element = node.GetView().as<xaml::UIElement>();
  const auto stableLeft = ViewPanel::GetLeft(element);
  const auto stableTop = ViewPanel::GetTop(element);

  // We cannot animate views that have an existing RenderTransform setting
  xaml::Media::TranslateTransform translateTransform = nullptr;
  if (const auto renderTransformProperty = element.GetValue(xaml::UIElement::RenderTransformProperty())) {
    translateTransform = renderTransformProperty.try_as<xaml::Media::TranslateTransform>();
    if (!translateTransform) {
      return nullptr;
    }
  }

  if (!translateTransform)
    translateTransform = {};

  const auto fromX = translateTransform.X();
  const auto fromY = translateTransform.Y();
  const auto currentLeft = stableLeft + fromX;
  const auto currentTop = stableTop + fromY;
  const auto toX = left - stableLeft;
  const auto toY = top - stableTop;
  const auto animateLocation = left != currentLeft || top != currentTop;

  if (animateLocation) {
    element.RenderTransform(translateTransform);
    xaml::Media::Animation::Storyboard storyboard;
    if (currentLeft != left)
      storyboard.Children().Append(
          CreateTimeline(element, L"(UIElement.RenderTransform).(TranslateTransform.X)", fromX, toX));

    if (currentTop != top)
      storyboard.Children().Append(
          CreateTimeline(element, L"(UIElement.RenderTransform).(TranslateTransform.Y)", fromY, toY));

    storyboard.Completed([weakView = winrt::make_weak(element), onComplete = std::move(onComplete)](auto &&...) {
      if (const auto view = weakView.get()) {
        view.RenderTransform(nullptr);
      }

      onComplete();
    });

    return storyboard;
  }

  return nullptr;
}

xaml::Media::Animation::DoubleAnimation
PaperUpdateLayoutAnimation::CreateTimeline(XamlView const &view, winrt::hstring path, double from, double to) {
  xaml::Media::Animation::DoubleAnimation timeline;
  timeline.From(from);
  timeline.To(to);
  timeline.Duration({TimeSpanFromMs(static_cast<double>(m_duration)), xaml::DurationType::TimeSpan});
  timeline.BeginTime({TimeSpanFromMs(static_cast<double>(m_delay))});
  timeline.EasingFunction(Interpolator());
  xaml::Media::Animation::Storyboard::SetTarget(timeline, view);
  xaml::Media::Animation::Storyboard::SetTargetProperty(timeline, path);
  return timeline;
}

#endif

}; // namespace react::uwp
