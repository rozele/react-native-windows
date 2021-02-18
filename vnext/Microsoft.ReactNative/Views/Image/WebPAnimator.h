#ifdef USE_WEBP

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "CppWinRTIncludes.h"
#include <../libwebp/webp/demux.h>

namespace react::uwp {

// TODO: pass in onLoad and onLoadError callbacks
class WebPAnimator : std::enable_shared_from_this<WebPAnimator> {
 public:
  WebPAnimator(
      winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> imageBrush,
      std::vector<uint8_t> &&buffer);

  ~WebPAnimator();

 private:
  winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> m_imageBrush;
  std::vector<uint8_t> m_buffer;

  WebPAnimDecoder *m_animDecoder;

  winrt::Windows::Foundation::DateTime m_loopStart;
  winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  int m_frameCount;
  int m_loopCount;
  int m_canvasWidth;
  int m_canvasHeight;

  int m_currentLoopIndex;
  int m_currentTimestamp;

  void DisplayNextFrame(
      winrt::Windows::Foundation::IInspectable const &sender,
      winrt::Windows::Foundation::IInspectable const &args);
};

} // namespace react::uwp

#else
class WebPAnimator {};
#endif
