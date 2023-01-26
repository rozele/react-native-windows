// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ScrollViewComponentView.h"

#include <UI.Xaml.Controls.h>
#include <Utils/ValueUtils.h>
#include <Views/Impl/ScrollViewUWPImplementation.h>

#pragma warning(push)
#pragma warning(disable : 4305)
#include <react/renderer/components/scrollview/ScrollViewShadowNode.h>
#pragma warning(pop)

#include <unicode.h>

namespace Microsoft::ReactNative {

ScrollViewComponentView::ScrollViewComponentView() {
  static auto const defaultProps = std::make_shared<facebook::react::ScrollViewProps const>();
  m_props = defaultProps;

  const auto snapPointManager = SnapPointManagingContentControl::Create();
  m_element.Content(*snapPointManager);
  snapPointManager->Content(m_contentPanel);

  m_element.HorizontalScrollBarVisibility(xaml::Controls::ScrollBarVisibility::Auto);
  m_element.VerticalScrollBarVisibility(xaml::Controls::ScrollBarVisibility::Auto);
  m_element.VerticalSnapPointsAlignment(xaml::Controls::Primitives::SnapPointsAlignment::Near);
  m_element.VerticalSnapPointsType(xaml::Controls::SnapPointsType::Mandatory);
  m_element.HorizontalSnapPointsType(xaml::Controls::SnapPointsType::Mandatory);
  m_contentPanel.VerticalAlignment(xaml::VerticalAlignment::Top);
  m_contentPanel.HorizontalAlignment(xaml::HorizontalAlignment::Left);

  const auto scrollViewUWPImplementation = ScrollViewUWPImplementation(m_element);
  m_scrollViewerSizeChangedRevoker =
      m_element.SizeChanged(winrt::auto_revoke, [scrollViewUWPImplementation](const auto &, const auto &) {
        scrollViewUWPImplementation.UpdateScrollableSize();
      });

  m_contentSizeChangedRevoker = scrollViewUWPImplementation.ScrollViewerSnapPointManager()->SizeChanged(
      winrt::auto_revoke, [this, scrollViewUWPImplementation](const auto &, const auto &) {
        scrollViewUWPImplementation.UpdateScrollableSize();
      });

  m_scrollViewerViewChangedRevoker = m_element.ViewChanged(
      winrt::auto_revoke, [this, scrollViewUWPImplementation](const auto &sender, const auto &args) {
        const auto scrollViewerNotNull{sender.as<winrt::ScrollViewer>()};
        const auto zoomFactor{scrollViewerNotNull.ZoomFactor()};
        if (m_zoomFactor != zoomFactor) {
          m_zoomFactor = zoomFactor;
          scrollViewUWPImplementation.UpdateScrollableSize();
        }
      });

  m_scrollViewerViewChangingRevoker =
      m_element.ViewChanging(winrt::auto_revoke, [this](const auto &sender, const auto &args) {
        const auto scrollViewerNotNull = sender.as<xaml::Controls::ScrollViewer>();

        facebook::react::ScrollViewMetrics scrollMetrics;
        scrollMetrics.containerSize.height = static_cast<facebook::react::Float>(m_element.ActualHeight());
        scrollMetrics.containerSize.width = static_cast<facebook::react::Float>(m_element.ActualWidth());
        scrollMetrics.contentOffset.x = static_cast<facebook::react::Float>(args.NextView().HorizontalOffset());
        scrollMetrics.contentOffset.y = static_cast<facebook::react::Float>(args.NextView().VerticalOffset());
        scrollMetrics.zoomScale = args.NextView().ZoomFactor();
        scrollMetrics.contentSize.height = static_cast<facebook::react::Float>(m_contentPanel.ActualHeight());
        scrollMetrics.contentSize.width = static_cast<facebook::react::Float>(m_contentPanel.ActualWidth());

        // If we are transitioning to inertial scrolling.
        if (m_isScrolling && !m_isScrollingFromInertia && args.IsInertial()) {
          m_isScrollingFromInertia = true;

          if (m_eventEmitter) {
            std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
                ->onScrollEndDrag(scrollMetrics);
            std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
                ->onMomentumScrollBegin(scrollMetrics);
          }
        }

        if (m_eventEmitter) {
          std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
              ->onScroll(scrollMetrics);
        }
      });

  m_scrollViewerDirectManipulationStartedRevoker =
      m_element.DirectManipulationStarted(winrt::auto_revoke, [this](const auto &sender, const auto &) {
        m_isScrolling = true;

        /*
        if (m_dismissKeyboardOnDrag && m_SIPEventHandler) {
          m_SIPEventHandler->TryHide();
        }
        */

        facebook::react::ScrollViewMetrics scrollMetrics;
        scrollMetrics.containerSize.height = static_cast<facebook::react::Float>(m_element.ActualHeight());
        scrollMetrics.containerSize.width = static_cast<facebook::react::Float>(m_element.ActualWidth());
        scrollMetrics.contentOffset.x = static_cast<facebook::react::Float>(m_element.HorizontalOffset());
        scrollMetrics.contentOffset.y = static_cast<facebook::react::Float>(m_element.VerticalOffset());
        scrollMetrics.zoomScale = m_element.ZoomFactor();
        scrollMetrics.contentSize.height = static_cast<facebook::react::Float>(m_contentPanel.ActualHeight());
        scrollMetrics.contentSize.width = static_cast<facebook::react::Float>(m_contentPanel.ActualWidth());

        const auto scrollViewer = sender.as<xaml::Controls::ScrollViewer>();
        if (m_eventEmitter) {
          std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
              ->onScrollBeginDrag(scrollMetrics);
        }
      });

  m_scrollViewerDirectManipulationCompletedRevoker =
      m_element.DirectManipulationCompleted(winrt::auto_revoke, [this](const auto &sender, const auto &args) {
        const auto scrollViewer = sender.as<xaml::Controls::ScrollViewer>();

        facebook::react::ScrollViewMetrics scrollMetrics;
        scrollMetrics.containerSize.height = static_cast<facebook::react::Float>(m_element.ActualHeight());
        scrollMetrics.containerSize.width = static_cast<facebook::react::Float>(m_element.ActualWidth());
        scrollMetrics.contentOffset.x = static_cast<facebook::react::Float>(m_element.HorizontalOffset());
        scrollMetrics.contentOffset.y = static_cast<facebook::react::Float>(m_element.VerticalOffset());
        scrollMetrics.zoomScale = m_element.ZoomFactor();
        scrollMetrics.contentSize.height = static_cast<facebook::react::Float>(m_contentPanel.ActualHeight());
        scrollMetrics.contentSize.width = static_cast<facebook::react::Float>(m_contentPanel.ActualWidth());

        if (m_eventEmitter) {
          if (m_isScrollingFromInertia) {
            std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
                ->onMomentumScrollEnd(scrollMetrics);
          } else {
            std::static_pointer_cast<facebook::react::ScrollViewEventEmitter const>(m_eventEmitter)
                ->onScrollEndDrag(scrollMetrics);
          }
        }

        m_isScrolling = false;
        m_isScrollingFromInertia = false;
      });
}

std::vector<facebook::react::ComponentDescriptorProvider>
ScrollViewComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void ScrollViewComponentView::mountChildComponentView(
    const IComponentView &childComponentView,
    uint32_t index) noexcept {
  m_contentPanel.Children().InsertAt(index, static_cast<const BaseComponentView &>(childComponentView).Element());
}

void ScrollViewComponentView::unmountChildComponentView(
    const IComponentView &childComponentView,
    uint32_t index) noexcept {
  m_contentPanel.Children().RemoveAt(index);
}

void ScrollViewComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ScrollViewProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ScrollViewProps>(props);

  if (oldViewProps.backgroundColor != newViewProps.backgroundColor) {
    if (newViewProps.backgroundColor) {
      m_element.Background(newViewProps.backgroundColor.AsWindowsBrush());
    } else {
      m_element.ClearValue(xaml::Controls::Control::BackgroundProperty());
    }
  }

  if (oldViewProps.scrollEnabled != newViewProps.scrollEnabled) {
    m_needsScrollModeUpdate = true;
  }

  if (oldViewProps.showsHorizontalScrollIndicator != newViewProps.showsHorizontalScrollIndicator) {
    const auto scrollBarVisibility = newViewProps.showsHorizontalScrollIndicator
        ? xaml::Controls::ScrollBarVisibility::Auto
        : xaml::Controls::ScrollBarVisibility::Hidden;
    m_element.HorizontalScrollBarVisibility(scrollBarVisibility);
  }

  if (oldViewProps.showsVerticalScrollIndicator != newViewProps.showsVerticalScrollIndicator) {
    const auto scrollBarVisibility = newViewProps.showsVerticalScrollIndicator
        ? xaml::Controls::ScrollBarVisibility::Auto
        : xaml::Controls::ScrollBarVisibility::Hidden;
    m_element.VerticalScrollBarVisibility(scrollBarVisibility);
  }

  if (oldViewProps.zoomScale != newViewProps.zoomScale) {
    if (m_element.IsLoaded()) {
      UpdateZoomScale(m_element, newViewProps.zoomScale);
    } else {
      // If the ScrollViewer has not yet Loaded, calling ChangeView will not work.
      // Thus, we defer the action until the Loaded event fires.
      m_controlLoadedRevoker = m_element.Loaded(
          winrt::auto_revoke, [this, zoomScale = newViewProps.zoomScale](winrt::IInspectable const &sender, auto &&) {
            const auto scrollViewer = sender.as<xaml::Controls::ScrollViewer>();
            UpdateZoomScale(m_element, zoomScale);
            m_controlLoadedRevoker.revoke();
          });
    }
  }

  if (oldViewProps.minimumZoomScale != newViewProps.minimumZoomScale) {
    m_element.MinZoomFactor(newViewProps.minimumZoomScale);
  }

  if (oldViewProps.maximumZoomScale != newViewProps.maximumZoomScale) {
    m_element.MaxZoomFactor(newViewProps.maximumZoomScale);
  }

  auto impl = ScrollViewUWPImplementation(m_element);
  if (oldViewProps.snapToStart != newViewProps.snapToStart) {
    impl.ScrollViewerSnapPointManager()->SnapToStart(newViewProps.snapToStart);
  }

  if (oldViewProps.snapToEnd != newViewProps.snapToEnd) {
    impl.ScrollViewerSnapPointManager()->SnapToEnd(newViewProps.snapToEnd);
  }

  if (oldViewProps.snapToAlignment != newViewProps.snapToAlignment) {
    switch (newViewProps.snapToAlignment) {
      case facebook::react::ScrollViewSnapToAlignment::End: {
        impl.SnapPointAlignment(xaml::Controls::Primitives::SnapPointsAlignment::Far);
        break;
      }
      case facebook::react::ScrollViewSnapToAlignment::Center: {
        impl.SnapPointAlignment(xaml::Controls::Primitives::SnapPointsAlignment::Center);
        break;
      }
      default: {
        impl.SnapPointAlignment(xaml::Controls::Primitives::SnapPointsAlignment::Near);
        break;
      }
    }
  }

  if (oldViewProps.snapToInterval != newViewProps.snapToInterval) {
    impl.ScrollViewerSnapPointManager()->SnapToInterval(newViewProps.snapToInterval);
  }

  if (oldViewProps.snapToOffsets != newViewProps.snapToOffsets) {
    const auto offsets = winrt::single_threaded_vector<float>();
    for (const auto offset : newViewProps.snapToOffsets) {
      offsets.Append(offset);
    }
    impl.ScrollViewerSnapPointManager()->SnapToOffsets(offsets.GetView());
  }

  if (oldViewProps.pagingEnabled != newViewProps.pagingEnabled) {
    impl.PagingEnabled(newViewProps.pagingEnabled);
  }

  Super::updateProps(props, oldProps);
}

void ScrollViewComponentView::updateEventEmitter(facebook::react::EventEmitter::Shared const &eventEmitter) noexcept {}
void ScrollViewComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {
  const auto &newState = *std::static_pointer_cast<facebook::react::ScrollViewShadowNode::ConcreteState const>(state);

  auto contentSize = newState.getData().getContentSize();
  m_contentPanel.Height(contentSize.height);
  m_contentPanel.Width(contentSize.width);
  m_contentSize = contentSize;
  m_needsScrollModeUpdate = true;
}
void ScrollViewComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  Super::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
  m_layoutMetrics = layoutMetrics;
  m_needsScrollModeUpdate = true;
}

void ScrollViewComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  if (m_needsScrollModeUpdate) {
    const auto &props = *std::static_pointer_cast<const facebook::react::ScrollViewProps>(m_props);
    const auto canScrollHorizontal = props.scrollEnabled && m_contentSize.width > m_layoutMetrics.frame.size.width;
    const auto canScrollVertical = props.scrollEnabled && m_contentSize.height > m_layoutMetrics.frame.size.height;
    m_element.HorizontalScrollMode(
        canScrollHorizontal ? xaml::Controls::ScrollMode::Auto : xaml::Controls::ScrollMode::Disabled);
    m_element.VerticalScrollMode(
        canScrollVertical ? xaml::Controls::ScrollMode::Auto : xaml::Controls::ScrollMode::Disabled);
    ScrollViewUWPImplementation(m_element).SetHorizontal(canScrollHorizontal && !canScrollVertical);
    m_needsScrollModeUpdate = false;
  }
}

void ScrollViewComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement ScrollViewComponentView::Element() const noexcept {
  return m_element;
}

void ScrollViewComponentView::UpdateZoomScale(xaml::Controls::ScrollViewer const &scrollViewer, float zoomScale) {
  // We want to keep a fixed center point. The current center point is given by:
  // let h = view port height
  // let y = scaled vertical offset
  // let z = zoom factor
  // h / (z * 2) + y / z
  //
  // We want the center point to remain unchanged with zoom, so we have to
  // solve for y' in the following equality:
  // let z' = target zoom factor
  // h / (z * 2) + y / z = h / (z' * 2) + y' / z'
  //
  // This gives us:
  // let r = z' / z
  // y' = (r - 1) * h / 2 + r * y
  //
  // We can calculate x' by following the approach above, substituting "h" for
  // the view port width and "y" for the scaled horizontal offset.
  winrt::IReference<double> xOffset = nullptr;
  winrt::IReference<double> yOffset = nullptr;
  const auto w = scrollViewer.ActualWidth();
  const auto h = scrollViewer.ActualHeight();
  const auto x = scrollViewer.HorizontalOffset();
  const auto y = scrollViewer.VerticalOffset();
  const auto z = scrollViewer.ZoomFactor();
  const auto r = zoomScale / z;
  xOffset = (r - 1) * w / 2 + r * x;
  yOffset = (r - 1) * h / 2 + r * y;
  scrollViewer.ChangeView(xOffset, yOffset, zoomScale);
}

} // namespace Microsoft::ReactNative
