// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <UI.Xaml.Controls.h>
#include "ScrollViewUWPImplementation.h"
#include "ScrollViewViewChanger.h"
#include "SnapPointManagingContentControl.h"

namespace react::uwp {

constexpr const double SCROLL_EPSILON = 1.0;

void ScrollViewViewChanger::Horizontal(bool horizontal) {
  m_horizontal = horizontal;
}

void ScrollViewViewChanger::Inverted(bool inverted) {
  m_inverted = inverted;
}

void ScrollViewViewChanger::ScrollToEnd(const xaml::Controls::ScrollViewer &scrollViewer, bool animated) {
  m_shouldRestartScrollToEnd = animated && m_inverted;
  if (m_shouldRestartScrollToEnd) {
    // Disable scroll anchoring while scrolling to end
    ScrollViewUWPImplementation(scrollViewer).SetContentAnchoringEnabled(false);
    SetContentScrollAnchors(scrollViewer, false);
  }

  if (m_horizontal) {
    m_targetScrollToEndOffset = scrollViewer.ScrollableWidth();
    scrollViewer.ChangeView(scrollViewer.ScrollableWidth(), nullptr, nullptr, !animated);
  } else {
    m_targetScrollToEndOffset = scrollViewer.ScrollableHeight();
    scrollViewer.ChangeView(nullptr, scrollViewer.ScrollableHeight(), nullptr, !animated);
  }
}

void ScrollViewViewChanger::OnViewChanging(
    const xaml::Controls::ScrollViewer &scrollViewer,
    const xaml::Controls::ScrollViewerViewChangingEventArgs &args) {
  // For non-inverted views, a ScrollViewer.ViewChanging event always emits an `onScroll` event
  if (m_inverted) {
    // For inverted views, we need to detect if we're scrolling to or away from the bottom edge to enable or disable
    // view anchoring
    const auto scrollingToEnd =
        IsScrollingToEnd(scrollViewer, args.NextView().HorizontalOffset(), args.NextView().VerticalOffset());

    // Do not update scroll anchoring while scrolling to end
    if (!m_shouldRestartScrollToEnd) {
      ScrollViewUWPImplementation(scrollViewer).SetContentAnchoringEnabled(!scrollingToEnd);
      if (m_wasViewAnchoringEnabled && scrollingToEnd) {
        // If scrolling to anchor edge, turn off view anchoring
        SetContentScrollAnchors(scrollViewer, false);
        m_wasViewAnchoringEnabled = false;
      } else if (!m_wasViewAnchoringEnabled && !scrollingToEnd) {
        // If scrolling away from anchor edge, turn on view anchoring
        SetContentScrollAnchors(scrollViewer, true);
        m_wasViewAnchoringEnabled = true;
      }
    }

    RestartScrollToEndIfNeeded(scrollViewer);
  }
}

void ScrollViewViewChanger::OnViewChanged(
    const xaml::Controls::ScrollViewer &scrollViewer,
    const xaml::Controls::ScrollViewerViewChangedEventArgs &args) {
  // Stop tracking scroll-to-end once the ScrollView comes to rest
  if (!args.IsIntermediate() && m_shouldRestartScrollToEnd) {
    m_shouldRestartScrollToEnd = false;
    if (!IsScrollingToEnd(scrollViewer, scrollViewer.HorizontalOffset(), scrollViewer.VerticalOffset())) {
      ScrollViewUWPImplementation(scrollViewer).SetContentAnchoringEnabled(true);
      SetContentScrollAnchors(scrollViewer, true);
    }
  }
}

/*static*/ void ScrollViewViewChanger::SetContentScrollAnchors(
    const xaml::Controls::ScrollViewer &scrollViewer,
    bool enabled) {
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

bool ScrollViewViewChanger::IsScrollingToEnd(
    const xaml::Controls::ScrollViewer &scrollViewer,
    double horizontalOffset,
    double verticalOffset) {
  return m_horizontal ? horizontalOffset > (scrollViewer.ScrollableWidth() - SCROLL_EPSILON)
                      : verticalOffset > (scrollViewer.ScrollableHeight() - SCROLL_EPSILON);
}

void ScrollViewViewChanger::RestartScrollToEndIfNeeded(const xaml::Controls::ScrollViewer &scrollViewer) {
  if (m_shouldRestartScrollToEnd) {
    if (m_horizontal && m_targetScrollToEndOffset != scrollViewer.ScrollableWidth()) {
      m_targetScrollToEndOffset = scrollViewer.ScrollableWidth();
      scrollViewer.ChangeView(scrollViewer.ScrollableWidth(), nullptr, nullptr);
    } else if (!m_horizontal && m_targetScrollToEndOffset != scrollViewer.ScrollableHeight()) {
      m_targetScrollToEndOffset = scrollViewer.ScrollableHeight();
      scrollViewer.ChangeView(nullptr, scrollViewer.ScrollableHeight(), nullptr);
    }
  }
}

} // namespace react::uwp
