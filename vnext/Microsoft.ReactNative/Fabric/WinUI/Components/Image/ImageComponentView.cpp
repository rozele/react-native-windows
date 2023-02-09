// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ImageComponentView.h"

#include <UI.Xaml.Controls.h>
#include <Utils/ValueUtils.h>

#include <IReactContext.h>

#pragma warning(push)
#pragma warning(disable : 4244 4305)
#include <react/renderer/components/image/ImageProps.h>
#pragma warning(pop)
#include <react/renderer/components/image/ImageEventEmitter.h>

namespace Microsoft::ReactNative {

ImageComponentView::ImageComponentView(winrt::Microsoft::ReactNative::ReactContext const &reactContext)
    : m_context(reactContext), m_element(ReactImage::Create()) {
  static auto const defaultProps = std::make_shared<facebook::react::ImageProps const>();
  m_props = defaultProps;
}

ImageComponentView::~ImageComponentView() {
  m_element->OnLoadEnd(m_onLoadEndToken);
}

std::vector<facebook::react::ComponentDescriptorProvider>
ImageComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void ImageComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldImageProps = *std::static_pointer_cast<const facebook::react::ImageProps>(m_props);
  const auto &newImageProps = *std::static_pointer_cast<const facebook::react::ImageProps>(props);

  if (oldImageProps.sources != newImageProps.sources) {
    if (newImageProps.sources.empty()) {
      // TODO clear image
    } else {
      ReactImageSource ris;
      ris.uri = newImageProps.sources[0].uri;
      ris.width = newImageProps.sources[0].size.width;
      ris.height = newImageProps.sources[0].size.height;
      ris.scale = newImageProps.sources[0].scale;

      auto contextSelf =
          winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactContext>(m_context.Handle());
      ris.bundleRootPath = contextSelf->GetInner().SettingsSnapshot().BundleRootPath();

      // TODO headers, method, __packager_asset?
      // Assume packager_asset for now...
      if (/*packager_assert && */ ris.uri.find("file://") == 0) {
        ris.uri.replace(0, 7, ris.bundleRootPath);
      }

      // Delay this until finalizeUpdates since the event emitter isn't set until after initial updateProps
      m_needsOnLoadStart = true;
      m_element->Source(ris);
    }
  }

  if (oldImageProps.blurRadius != newImageProps.blurRadius) {
    m_element->BlurRadius(newImageProps.blurRadius);
  }

  if (oldImageProps.tintColor != newImageProps.tintColor) {
    if (newImageProps.tintColor) {
      m_element->TintColor(newImageProps.tintColor.AsWindowsColor());
    } else {
      m_element->TintColor(winrt::Colors::Transparent());
    }
  }

  if (oldImageProps.resizeMode != newImageProps.resizeMode) {
    m_element->ResizeMode(newImageProps.resizeMode);
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

void ImageComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  if (m_needsOnLoadStart) {
    std::static_pointer_cast<const facebook::react::ImageEventEmitter>(m_eventEmitter)->onLoadStart();
    m_needsOnLoadStart = false;
  }
}

void ImageComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement ImageComponentView::Element() const noexcept {
  return *m_element;
}

} // namespace Microsoft::ReactNative
