// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "WebPAnimator.h"

#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>

namespace winrt {
  using namespace Windows::Foundation;
  using namespace Windows::Graphics::Imaging;
  using namespace Windows::Storage::Streams;
  using namespace Windows::UI::Xaml::Media::Imaging;
} // namespace winrt

namespace react::uwp {

winrt::IAsyncAction WebPAnimator::InitializeAsync(winrt::IRandomAccessStream const &memoryStream) {
  // TODO: use libwebp, this isn't supported
  m_bitmapDecoder = co_await winrt::BitmapDecoder::CreateAsync(memoryStream);

  auto currentImage = co_await m_bitmapDecoder.GetSoftwareBitmapAsync(
      winrt::BitmapPixelFormat::Bgra8, winrt::BitmapAlphaMode::Premultiplied);
  auto imageBrush{m_imageBrush.get()};
  auto imageSource = imageBrush.ImageSource().try_as<winrt::SoftwareBitmapSource>();
  if (!imageSource) {
    imageSource = winrt::SoftwareBitmapSource{};
    imageBrush.ImageSource(imageSource);
  }

  co_await imageSource.SetBitmapAsync(std::move(currentImage));

  auto frameCount{m_bitmapDecoder.FrameCount()};
  if (frameCount > 1) {
    m_dispatcherTimer.Interval(std::chrono::milliseconds(150));
    m_tickRevoker = m_dispatcherTimer.Tick(winrt::auto_revoke, { this, &WebPAnimator::UpdateImageBrush });
    m_dispatcherTimer.Start();
  }
}

WebPAnimator::~WebPAnimator() {
  m_tickRevoker.revoke();
  m_dispatcherTimer.Stop();
}

winrt::fire_and_forget WebPAnimator::UpdateImageBrush(winrt::IInspectable const &sender, winrt::IInspectable const &args) {
  if (auto imageBrush = m_imageBrush.get()) {
    if (currentFrameIndex == m_bitmapDecoder.FrameCount()) {
      currentFrameIndex = 0;
    }

    // TODO: Conditional check shouldn't be needed
    if (auto imageSource = imageBrush.ImageSource().try_as<winrt::SoftwareBitmapSource>()) {
      auto frame{co_await m_bitmapDecoder.GetFrameAsync(currentFrameIndex++)};
      auto bitmap{co_await frame.GetSoftwareBitmapAsync(
          winrt::BitmapPixelFormat::Bgra8, winrt::BitmapAlphaMode::Premultiplied)};
      co_await imageSource.SetBitmapAsync(bitmap);
    }
  }
}

} // namespace react::uwp
