#ifdef USE_WEBP

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "WebPAnimator.h"

#include <winrt/Windows.UI.Xaml.Media.Imaging.h>

namespace winrt {
  using namespace Windows::Foundation;
  using namespace Windows::UI::Xaml::Media;
  using namespace Windows::UI::Xaml::Media::Imaging;
} // namespace winrt

namespace react::uwp {

winrt::WriteableBitmap CreateNextFrame(int width, int height, WebPAnimDecoder* decoder, int* timestamp);

WebPAnimator::WebPAnimator(winrt::weak_ref<winrt::ImageBrush> imageBrush, std::vector<uint8_t>&& buffer) {
  m_imageBrush = imageBrush;

  // Take ownership of WebP bitstream
  m_buffer = std::move(buffer);

  // Initialize WebP container
  WebPData webpData;
  webpData.bytes = m_buffer.data();
  webpData.size = m_buffer.size();

  // Create demuxer to read WebP metadata
  auto demuxer = WebPDemux(&webpData);
  // TODO: use has_animation feature rather than frame_count?
  m_frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
  m_canvasWidth = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH);
  m_canvasHeight = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT);
  m_loopCount = 3; //WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT);
  WebPDemuxDelete(demuxer);

  // Create WebPAnimDecoder
  WebPAnimDecoderOptions decOptions;
  WebPAnimDecoderOptionsInit(&decOptions);
  decOptions.color_mode = MODE_bgrA;
  decOptions.use_threads = 1;
  m_animDecoder = WebPAnimDecoderNew(&webpData, &decOptions);

  if (m_frameCount == 1) {
    // Not animated
    auto strongBrush = m_imageBrush.get();
    if (strongBrush) {
      int ignored;
      // Render first and only frame
      strongBrush.ImageSource(CreateNextFrame(m_canvasWidth, m_canvasHeight, m_animDecoder, &ignored));
    }
  } else if (m_frameCount > 1) {
    // The CompositionTarget rendering event is called once per frame after layout has been computed.
    // This feels like a heavyweight approach, but DispatcherTimer resulted in slow animations.
    m_loopStart = winrt::clock::now();
    m_renderingRevoker =
        winrt::CompositionTarget::Rendering(winrt::auto_revoke, {this, &WebPAnimator::DisplayNextFrame});
  }
}

WebPAnimator::~WebPAnimator() {
  if (m_frameCount > 1) {
    m_renderingRevoker.revoke();
  }

  // TODO: we could potentially free the WebPAnimDecoder more eagerly for finite loops or static images
  WebPAnimDecoderDelete(m_animDecoder);
}

void WebPAnimator::DisplayNextFrame(winrt::IInspectable const &sender, winrt::IInspectable const &args) {
  // Return if frame duration has not been reached
  if ((winrt::clock::now() - m_loopStart) < std::chrono::milliseconds(m_currentTimestamp)) {
    return;
  }

  if (auto imageBrush = m_imageBrush.get()) {
    if (!WebPAnimDecoderHasMoreFrames(m_animDecoder)) {
      if (m_loopCount == 0 || ++m_currentLoopIndex < m_loopCount) {
        WebPAnimDecoderReset(m_animDecoder);
        m_loopStart = winrt::clock::now();
      } else {
        // Unsubscribe from the rendering event, this call is idempotent so
        // there are no concerns with it being called again in the destructor.
        m_renderingRevoker.revoke();
        return;
      }
    }
    // Otherwise we need to unsubscribe

    // Create and display next frame
    int timestamp;
    imageBrush.ImageSource(CreateNextFrame(m_canvasWidth, m_canvasHeight, m_animDecoder, &timestamp));

    // The difference between the previous frames timestamp and the current frames timestamp is the duration
    m_currentTimestamp = timestamp;
  }
}

winrt::WriteableBitmap CreateNextFrame(int width, int height, WebPAnimDecoder* decoder, int* timestamp) {
  // Render next frame to internal animation buffer
  // Internal animation buffer is owned by WebpAnimDecoder
  uint8_t *buf;
  WebPAnimDecoderGetNext(decoder, &buf, timestamp);

  // Copy frame to WriteableBitmap
  // TODO: should we pool these bitmaps, or potentially swap between two instances?
  winrt::WriteableBitmap writeableBitmap{width, height};
  auto pixels = writeableBitmap.PixelBuffer().data();
  memcpy(pixels, buf, width * height * sizeof(uint32_t));

  return writeableBitmap;
}

} // namespace react::uwp

#endif
