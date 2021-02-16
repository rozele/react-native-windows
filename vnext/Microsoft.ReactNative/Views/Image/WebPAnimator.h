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

class WebPAnimator : public std::enable_shared_from_this<WebPAnimator> {
 public:
  WebPAnimator(winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> imageBrush)
      : m_weakBrush{imageBrush} {};

  winrt::Windows::Foundation::IAsyncOperation<bool> SetSourceAsync(winrt::Windows::Storage::Streams::IRandomAccessStream inputStream);

  void Start();

  bool IsAnimated() {
    return m_frames.size() > 1;
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

  // Handle used to revoke CompositionTarget::Rendering registration
  winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  // WebP canvas width
  int m_canvasWidth{0};
  // WebP canvas height
  int m_canvasHeight{0};
  // WebP loop count
  int m_loopCount{0};
  // WebP frame data
  std::vector<winrt::Windows::UI::Xaml::Media::Imaging::WriteableBitmap> m_frames;
  // WebP frame timestamps
  std::vector<int> m_timestamps;

  // Current animation frame start
  winrt::Windows::Foundation::DateTime m_frameStart;
  // Current frame index
  int m_frameIndex{0};
  // Current loop index
  int m_loopIndex{0};

  void DisplayNextFrame();
};

} // namespace react::uwp

#else
namespace react::uwp {
class WebPAnimator {};
} // namespace react::uwp
#endif
