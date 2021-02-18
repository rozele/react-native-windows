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
      std::vector<uint8_t> &&buffer,
      std::function<void(bool)>&& onLoadEnd);

  ~WebPAnimator();

 private:
  winrt::weak_ref<winrt::Windows::UI::Xaml::Media::ImageBrush> m_imageBrush;
  std::vector<uint8_t> m_buffer;
  std::function<void(bool)> m_onLoadEnd;

  WebPAnimDecoder *m_animDecoder;

  winrt::Windows::Foundation::DateTime m_loopStart;
  winrt::Windows::UI::Xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  int m_frameCount;
  int m_loopCount;
  int m_canvasWidth;
  int m_canvasHeight;

  bool m_invokedOnLoadEnd;
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
