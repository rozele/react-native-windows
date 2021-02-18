#ifdef USE_WEBP

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CppWinRTIncludes.h"
#include <webp/demux.h>

namespace react::uwp {

class WebPAnimator {
 public:
  WebPAnimator(
      winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> imageBrush,
      std::function<void(bool)> &&onLoadEnd)
      : m_imageBrush{imageBrush}, m_onLoadEnd{onLoadEnd} {};

  winrt::Windows::Foundation::IAsyncAction SetSourceAsync(winrt::Windows::Storage::Streams::IRandomAccessStream inputStream);

  bool IsAnimated() {
    return m_frameCount > 1;
  };

  ~WebPAnimator();

 private:
  winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> m_imageBrush;
  std::function<void(bool)> m_onLoadEnd;

  winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  std::vector<uint8_t> m_buffer;
  WebPAnimDecoder *m_animDecoder;
  uint8_t *m_nextFrameData;
  int m_nextTimestamp;

  int m_frameCount;
  int m_loopCount;
  int m_canvasWidth;
  int m_canvasHeight;

  winrt::Windows::Foundation::IAsyncAction m_createNextFrameTask;
  winrt::Windows::Foundation::DateTime m_frameStart;
  int m_currentTimestamp;
  int m_currentDuration;

  bool m_invokedOnLoadEnd;
  int m_currentLoopIndex;
  
  winrt::Windows::Foundation::IAsyncAction CreateNextFrameAsync();
  void DisplayNextFrame(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Windows::Foundation::IInspectable const &args);
};

} // namespace react::uwp

#else
class WebPAnimator {};
#endif
