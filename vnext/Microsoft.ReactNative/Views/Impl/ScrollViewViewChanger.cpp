// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <UI.Xaml.Controls.h>
#include "ScrollViewViewChanger.h"
#include "ScrollViewUWPImplementation.h"
#include "SnapPointManagingContentControl.h"

namespace react::uwp {

constexpr const double SCROLL_EPSILON = 1.0;

void ScrollViewViewChanger::Horizontal(bool horizontal) {
  m_horizontal = horizontal;
}

bool ScrollViewViewChanger::Inverted() const {
  return m_inverted;
}

void ScrollViewViewChanger::Inverted(bool inverted) {
  m_inverted = inverted;
}

std::tuple<double, double>
ScrollViewViewChanger::GetScrollOffsets(xaml::Controls::ScrollViewer scrollViewer, double x, double y) {
  // Compute the inverted scroll offsets if the inverted flag is set
  const auto adjustedX = m_horizontal && m_inverted ? scrollViewer.ScrollableWidth() - x : x;
  const auto adjustedY = !m_horizontal && m_inverted ? scrollViewer.ScrollableHeight() - y : y;
  return std::make_tuple(adjustedX, adjustedY);
}

void ScrollViewViewChanger::ChangeView(
    xaml::Controls::ScrollViewer scrollViewer,
    const winrt::IReference<double> x,
    const winrt::IReference<double> y,
    bool animated) {
  // If scrolling in primary axis, keep scroll command in sync with size changes
  m_activeScrollCommand = (m_horizontal && x != nullptr) || (!m_horizontal && y != nullptr);

  // Store the latest values in case the scroll command needs to be restarted
  m_lastX = x;
  m_lastY = y;
  m_lastAnimated = animated;

  // Calculate adjusted scroll offsets for inverted views
  const auto [adjustedX, adjustedY] =
      GetScrollOffsets(scrollViewer, x != nullptr ? x.Value() : 0, y != nullptr ? y.Value() : 0);

  // Set the target "final view" for detection of interrupted scroll command
  m_adjustedTargetX = x != nullptr ? adjustedX : scrollViewer.HorizontalOffset();
  m_adjustedTargetY = y != nullptr ? adjustedY : scrollViewer.VerticalOffset();

  scrollViewer.ChangeView(x == nullptr ? x : adjustedX, y == nullptr ? y : adjustedY, nullptr, !animated);
}

void ScrollViewViewChanger::OnSizeChanged(const xaml::Controls::ScrollViewer &scrollViewer) {
  // Restart scroll command if size changes when inverted
  if (m_inverted && m_activeScrollCommand) {
    ChangeView(scrollViewer, m_lastX, m_lastY, m_lastAnimated);
  }
}

void ScrollViewViewChanger::OnViewChanging(
    const xaml::Controls::ScrollViewer &scrollViewer,
    const xaml::Controls::ScrollViewerViewChangingEventArgs &args) {
  // If the scroll destination has changed, we can assume it's due to a user manipulation and scroll command is canceled
  const auto expectedOffset = m_horizontal ? m_adjustedTargetX : m_adjustedTargetY;
  const auto actualOffset = m_horizontal ? args.FinalView().HorizontalOffset() : args.FinalView().VerticalOffset();

  // For safety, checking if the target offset and the projected final offset are within epsilon (rather than checking
  // for equality).
  if (std::abs(expectedOffset - actualOffset) > SCROLL_EPSILON) {
    m_activeScrollCommand = false;
  }

  // For inverted views, we need to detect if we're scrolling to or away from the bottom edge
  if (m_inverted) {
    const auto [nextX, nextY] =
        GetScrollOffsets(scrollViewer, args.NextView().HorizontalOffset(), args.NextView().VerticalOffset());

    auto scrolledToTop = m_horizontal ? m_latestX < SCROLL_EPSILON : m_latestY < SCROLL_EPSILON;
    auto scrollingToTop = m_horizontal ? nextX < SCROLL_EPSILON : nextY < SCROLL_EPSILON;
    m_latestX = nextX;
    m_latestY = nextY;

    ScrollViewUWPImplementation(scrollViewer).SetScrolledToTop(scrollingToTop);
    if (scrolledToTop && !scrollingToTop) {
      // If scrolling away from anchor edge, turn on view anchoring
      SetContentScrollAnchors(scrollViewer, true);
    } else if (!scrolledToTop && scrollingToTop) {
      // If scrolling to anchor edge, turn off view anchoring
      SetContentScrollAnchors(scrollViewer, false);
    }
  }
}

void ScrollViewViewChanger::OnViewChanged(const xaml::Controls::ScrollViewerViewChangedEventArgs &args) {
  // Stop tracking scroll command once the ScrollView comes to rest
  if (!args.IsIntermediate()) {
    m_activeScrollCommand = false;
  }
}

void ScrollViewViewChanger::SetContentScrollAnchors(const xaml::Controls::ScrollViewer &scrollViewer, bool enabled) {
  if (auto snapPointManager = scrollViewer.Content().as<react::uwp::SnapPointManagingContentControl>()) {
    if (auto panel = snapPointManager->Content().as<xaml::Controls::Panel>()) {
      for (const auto &child : panel.Children()) {
        const auto &childElement = child.as<xaml::UIElement>();
        if (enabled) {
          childElement.CanBeScrollAnchor(true);
        } else {
          childElement.ClearValue(xaml::UIElement::CanBeScrollAnchorProperty());
        }
      }
    }
  }
}

} // namespace react::uwp
