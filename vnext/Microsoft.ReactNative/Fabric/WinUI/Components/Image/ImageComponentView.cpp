// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ImageComponentView.h"

#include <UI.Xaml.Controls.h>
#include <Utils/ResourceBrushUtils.h>
#include <Utils/ValueUtils.h>

#include <IReactContext.h>

#pragma warning(push)
#pragma warning(disable : 4244 4305)
#include <react/renderer/components/image/ImageProps.h>
#pragma warning(pop)
#include <react/renderer/components/image/ImageEventEmitter.h>

namespace Microsoft::ReactNative {

ImageComponentView::ImageComponentView() : m_element(ReactImage::Create()) {
  static auto const defaultProps = std::make_shared<facebook::react::ImageProps const>();
  m_props = defaultProps;
  m_element->ResizeMode(defaultProps->resizeMode);
}

ImageComponentView::~ImageComponentView() {
  m_element->OnLoadEnd(m_onLoadEndToken);
}

void ImageComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldImageProps = *std::static_pointer_cast<const facebook::react::ImageProps>(m_props);
  const auto &newImageProps = *std::static_pointer_cast<const facebook::react::ImageProps>(props);

  if (oldImageProps.sources != newImageProps.sources) {
    if (newImageProps.sources.empty()) {
      // TODO(T145214627): Clear image source
    } else {
      // TODO(T145212483): Wire up headers values when available
      ReactImageSource imageSource;
      imageSource.uri = newImageProps.sources[0].uri;
      imageSource.width = newImageProps.sources[0].size.width;
      imageSource.height = newImageProps.sources[0].size.height;
      imageSource.scale = newImageProps.sources[0].scale;

      // Delay this until finalizeUpdates since the event emitter isn't set until after initial updateProps
      m_needsOnLoadStart = true;
      m_element->Source(imageSource);
    }
  }

  if (oldImageProps.blurRadius != newImageProps.blurRadius) {
    m_element->BlurRadius(newImageProps.blurRadius);
  }

  if (oldImageProps.tintColor != newImageProps.tintColor) {
    if (newImageProps.tintColor) {
      m_element->TintColor(newImageProps.tintColor.AsWindowsColor());
    } else {
      m_element->TintColor(facebook::react::clearColor().AsWindowsColor());
    }
  }

  if (oldImageProps.resizeMode != newImageProps.resizeMode) {
    m_element->ResizeMode(newImageProps.resizeMode);
  }

  if (oldImageProps.borderColors != newImageProps.borderColors) {
    if (newImageProps.borderColors.all) {
      m_element->BorderBrush(newImageProps.borderColors.all->AsWindowsBrush());
    } else {
      m_element->ClearValue(xaml::Controls::Grid::BorderBrushProperty());
    }
  }

  if (oldImageProps.borderStyles != newImageProps.borderStyles ||
      oldImageProps.borderRadii != newImageProps.borderRadii) {
    m_needsBorderUpdate = true;
  }

  Super::updateProps(props, oldProps);
}

void ImageComponentView::updateEventEmitter(facebook::react::EventEmitter::Shared const &eventEmitter) noexcept {
  std::weak_ptr<facebook::react::EventEmitter const> weakEmitter = eventEmitter;
  m_element->OnLoadEnd(m_onLoadEndToken);
  m_onLoadEndToken = m_element->OnLoadEnd([weakEmitter](auto &&, const bool &succeeded) {
    if (const auto eventEmitter = weakEmitter.lock()) {
      const auto imageEventEmitter = std::static_pointer_cast<const facebook::react::ImageEventEmitter>(eventEmitter);
      if (succeeded) {
        imageEventEmitter->onLoad();
      } else {
        imageEventEmitter->onError();
      }
      imageEventEmitter->onLoadEnd();
    }
  });

  Super::updateEventEmitter(eventEmitter);
}

void ImageComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {}

void ImageComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  Super::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
  m_layoutMetrics = layoutMetrics;
  m_needsBorderUpdate = true;
}

void ImageComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  if (m_needsOnLoadStart && m_eventEmitter) {
    std::static_pointer_cast<const facebook::react::ImageEventEmitter>(m_eventEmitter)->onLoadStart();
    m_needsOnLoadStart = false;
  }

  if (m_needsBorderUpdate) {
    const auto &props = *std::static_pointer_cast<const facebook::react::ImageProps>(m_props);
    auto const borderMetrics = props.resolveBorderMetrics(m_layoutMetrics);
    m_element->BorderThickness(xaml::ThicknessHelper::FromLengths(
        borderMetrics.borderWidths.left,
        borderMetrics.borderWidths.top,
        borderMetrics.borderWidths.right,
        borderMetrics.borderWidths.bottom));

    xaml::CornerRadius cornerRadius;
    cornerRadius.BottomLeft = borderMetrics.borderRadii.bottomLeft;
    cornerRadius.BottomRight = borderMetrics.borderRadii.bottomRight;
    cornerRadius.TopLeft = borderMetrics.borderRadii.topLeft;
    cornerRadius.TopRight = borderMetrics.borderRadii.topRight;

    if (borderMetrics.borderWidths != facebook::react::BorderWidths{} &&
        m_element->ReadLocalValue(xaml::Controls::Grid::BorderBrushProperty()) ==
            xaml::DependencyProperty::UnsetValue()) {
      m_element->BorderBrush(DefaultBrushStore::Instance().GetDefaultBorderBrush());
    }

    m_element->CornerRadius(cornerRadius);

    m_needsBorderUpdate = false;
  }
}

void ImageComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement ImageComponentView::Element() const noexcept {
  return *m_element;
}

} // namespace Microsoft::ReactNative
