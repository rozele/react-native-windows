// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CppWinRTIncludes.h"

namespace react::uwp {

class WebPAnimator : std::enable_shared_from_this<WebPAnimator> {
 public:
  WebPAnimator(winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> imageBrush) : m_imageBrush{imageBrush} {}
  ~WebPAnimator();

  winrt::IAsyncAction InitializeAsync(const winrt::Windows::Storage::Streams::IRandomAccessStream& memoryStream);

 private:
  winrt::fire_and_forget UpdateImageBrush(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args);

  unsigned int currentFrameIndex = 1;
  winrt::Windows::Graphics::Imaging::BitmapDecoder m_bitmapDecoder = nullptr;
  winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> m_imageBrush;
  winrt::Windows::UI::Xaml::DispatcherTimer m_dispatcherTimer;
  winrt::event_revoker<winrt::Windows::UI::Xaml::IDispatcherTimer> m_tickRevoker;
};

} // namespace react::uwp
