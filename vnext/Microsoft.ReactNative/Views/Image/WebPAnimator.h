#ifdef USE_WEBP

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CppWinRTIncludes.h"
#include <webp/demux.h>

namespace react::uwp {

enum class WebPFrameState {
  Started = 0,
  Ready = 1,
  Canceled = 2,
  Failed = 3,
};

class WebPAnimator {
 public:
  WebPAnimator(
      winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> imageBrush,
      std::function<void(bool)> &&onLoadEnd)
      : m_weakBrush{imageBrush}, m_onLoadEnd{onLoadEnd} {};

  winrt::Windows::Foundation::IAsyncAction SetSourceAsync(winrt::Windows::Storage::Streams::IRandomAccessStream inputStream);

  bool IsAnimated() {
    return m_frameCount > 1;
  };

  int PixelWidth() {
    return m_canvasWidth;
  };

  int PixelHeight() {
    return m_canvasHeight;
  };

  ~WebPAnimator();

 private:
  // ImageBrush supplied via constructor
  winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> m_weakBrush;
  // Callback supplied by the constructor 
  std::function<void(bool)> m_onLoadEnd;

  // Handle used to revoke CompositionTarget::Rendering registration
  winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  // Mutex used to ensure we do not delete the WebPAnimationDecoder while it is in use
  std::mutex m_disposeMutex;
  // Flag that signals that the WebPAnimationDecoder is deleted
  bool m_isDisposed = false;

  // Byte buffer for WebP data
  std::vector<uint8_t> m_buffer;
  // Animation decoder from libwebp
  WebPAnimDecoder *m_animDecoder;

  // Canvas width of the WebP image
  int m_canvasWidth;
  // Canvas height of the WebP image
  int m_canvasHeight;
  // Number of frames in the WebP image
  int m_frameCount;
  // Number of times the WebP animation should loop
  int m_loopCount;
  // Number of times the WebP animation has actually looped
  int m_currentLoopIndex;

  // Pointer to frame data when it's finished decoding, memory owned by m_animDecoder
  uint8_t *m_nextFrameData;
  // Frame decoding state
  WebPFrameState m_nextFrameState;

  // Timestamp when the last frame was displayed, used to determine when next frame should be displayed
  winrt::Windows::Foundation::DateTime m_frameStart;
  // Duration of current frame, next frame will be displayed at m_frameStart + m_currentDuration
  int m_currentDurationMs = 0;
  // Timestamp of the currently displayed frame, used to calculate duration of next frame
  int m_currentTimestampMs = 0;
  // Timestamp of next frame, used to calculate duration of next frame
  int m_nextTimestampMs;

  winrt::Windows::Foundation::IAsyncAction CreateNextFrameAsync();
  void DisplayNextFrame(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Windows::Foundation::IInspectable const &args);
};

} // namespace react::uwp

#else
namespace react::uwp {
class WebPAnimator {};
} // namespace react::uwp
#endif
