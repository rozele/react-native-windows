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
  m_lastChangeViewX = x;
  m_lastChangeViewY = y;
  m_lastChangeViewAnimated = animated;

  // Calculate adjusted scroll offsets for inverted views
  const auto [adjustedX, adjustedY] =
      GetScrollOffsets(scrollViewer, x != nullptr ? x.Value() : 0, y != nullptr ? y.Value() : 0);

  scrollViewer.ChangeView(x == nullptr ? x : adjustedX, y == nullptr ? y : adjustedY, nullptr, !animated);
}

bool ScrollViewViewChanger::OnSizeChanged(const xaml::Controls::ScrollViewer &scrollViewer) {
  // In order to keep the current scroll position in sync with JS, we may need to emit
  // an `onScroll` event when the layout changes, even when the view port does not
  // When not inverted, we never need to emit the event
  if (!m_inverted) {
    return false;
  }

  // Restart scroll command if size changes when inverted
  if (m_activeScrollCommand) {
    ChangeView(scrollViewer, m_lastChangeViewX, m_lastChangeViewY, m_lastChangeViewAnimated);
    // Do not bother emitting an `onScroll` event as the `ChangeView` command will continue to emit events
    return false;
  }

  // Return a value indicating if the inverted offsets have changed
  const auto [nextX, nextY] =
      GetScrollOffsets(scrollViewer, scrollViewer.HorizontalOffset(), scrollViewer.VerticalOffset());
  return UpdateLatestOffsets(scrollViewer, nextX, nextY);
}

bool ScrollViewViewChanger::OnViewChanging(
    const xaml::Controls::ScrollViewer &scrollViewer,
    const xaml::Controls::ScrollViewerViewChangingEventArgs &args) {
  // For non-inverted views, a ScrollViewer.ViewChanging event always emits an `onScroll` event
  if (!m_inverted) {
    return true;
  }

  const auto [nextX, nextY] =
      GetScrollOffsets(scrollViewer, args.NextView().HorizontalOffset(), args.NextView().VerticalOffset());

  // If the inverted offsets have not changed, we should not emit an event
  // We also will not need to check if scrolling to or from the bottom edge
  if (!UpdateLatestOffsets(scrollViewer, nextX, nextY)) {
    return false;
  }

  // For inverted views, we need to detect if we're scrolling to or away from the bottom edge to enable or disable view anchoring
  auto scrolledToTop = m_horizontal ? m_latestOnScrollX < SCROLL_EPSILON : m_latestOnScrollY < SCROLL_EPSILON;
  auto scrollingToTop = m_horizontal ? nextX < SCROLL_EPSILON : nextY < SCROLL_EPSILON;
  m_latestOnScrollX = nextX;
  m_latestOnScrollY = nextY;

  ScrollViewUWPImplementation(scrollViewer).SetScrolledToTop(scrollingToTop);
  if (scrolledToTop && !scrollingToTop) {
    // If scrolling away from anchor edge, turn on view anchoring
    SetContentScrollAnchors(scrollViewer, true);
  } else if (!scrolledToTop && scrollingToTop) {
    // If scrolling to anchor edge, turn off view anchoring
    SetContentScrollAnchors(scrollViewer, false);
  }

  // The inverted offsets have changed an event should be emitted
  return true;
}

void ScrollViewViewChanger::OnViewChanged(const xaml::Controls::ScrollViewerViewChangedEventArgs &args) {
  // Stop tracking scroll command once the ScrollView comes to rest
  if (!args.IsIntermediate()) {
    m_activeScrollCommand = false;
  }
}

bool ScrollViewViewChanger::UpdateLatestOffsets(const xaml::Controls::ScrollViewer& scrollViewer, double x, double y) {
  if (std::abs(x - m_latestOnScrollX) > SCROLL_EPSILON || std::abs(y - m_latestOnScrollY) > SCROLL_EPSILON) {
    m_latestOnScrollX = x;
    m_latestOnScrollY = y;
    return true;
  }

  return false;
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
