#ifdef USE_WEBP

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "WebPAnimator.h"

#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>

namespace winrt {
  using namespace Windows::Foundation;
  using namespace Windows::Storage::Streams;
  using namespace Windows::UI::Xaml::Media;
  using namespace Windows::UI::Xaml::Media::Imaging;
} // namespace winrt

namespace react::uwp {

winrt::IAsyncAction WebPAnimator::SetSourceAsync(winrt::IRandomAccessStream inputStream) {
  // Copy stream contents to memory buffer
  auto length{static_cast<size_t>(inputStream.Size())};
  winrt::DataReader reader{inputStream};
  m_buffer.resize(length);
  co_await reader.LoadAsync(length);
  reader.ReadBytes(m_buffer);

  // Initialize WebP container
  WebPData webpData;
  webpData.bytes = m_buffer.data();
  webpData.size = m_buffer.size();

  // Create demuxer to read WebP metadata
  auto demuxer = WebPDemux(&webpData);
  if (!demuxer) {
    m_onLoadEnd(false);
    co_return;
  }

  // TODO: use has_animation feature rather than frame_count?
  m_frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
  m_canvasWidth = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH);
  m_canvasHeight = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT);
  m_loopCount = WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT);
  WebPDemuxDelete(demuxer);

  // Create WebPAnimDecoder
  WebPAnimDecoderOptions decOptions;
  if (!WebPAnimDecoderOptionsInit(&decOptions)) {
    m_onLoadEnd(false);
    co_return;
  }

  decOptions.color_mode = MODE_bgrA;
  m_animDecoder = WebPAnimDecoderNew(&webpData, &decOptions);

  if (m_frameCount == 1) {
    // Not animated
    auto imageBrush = m_imageBrush.get();
    if (imageBrush) {
      co_await CreateNextFrameAsync();

      if (m_nextFrameData) {
        // Copy frame data to WriteableBitmap and set source
        winrt::WriteableBitmap frame{m_canvasWidth, m_canvasHeight};
        memcpy(frame.PixelBuffer().data(), m_nextFrameData, m_canvasWidth * m_canvasHeight * sizeof(uint32_t));
        imageBrush.ImageSource(frame);
      }

      m_onLoadEnd(m_nextFrameData);
    }
  } else if (m_frameCount > 1) {
    // The CompositionTarget rendering event is called once per frame after layout has been computed.
    // This feels like a heavyweight approach, but DispatcherTimer resulted in slow animations.
    m_frameStart = winrt::clock::now();
    m_createNextFrameTask = CreateNextFrameAsync();
    m_renderingRevoker =
        winrt::CompositionTarget::Rendering(winrt::auto_revoke, {this, &WebPAnimator::DisplayNextFrame});
  }
}

WebPAnimator::~WebPAnimator() {
  if (m_frameCount > 1) {
    m_renderingRevoker.revoke();
  }

  // TODO: we could potentially free the WebPAnimDecoder more eagerly for finite loops
  WebPAnimDecoderDelete(m_animDecoder);
}

void WebPAnimator::DisplayNextFrame(winrt::IInspectable const &sender, winrt::IInspectable const &args) {
  // Return if frame duration has not been reached, or the next frame is not ready
  if ((winrt::clock::now() - m_frameStart) < std::chrono::milliseconds(m_currentDuration) || m_createNextFrameTask.Status() != winrt::AsyncStatus::Completed) {
    return;
  }

  if (auto imageBrush = m_imageBrush.get()) {
    // Check if we've reached the end of the loop
    if (!WebPAnimDecoderHasMoreFrames(m_animDecoder)) {
      if (m_loopCount == 0 || ++m_currentLoopIndex < m_loopCount) {
        // Reset the WebPAnimationDecoder to the first frame
        WebPAnimDecoderReset(m_animDecoder);
      } else {
        // Unsubscribe from the rendering event, this call is idempotent so
        // there are no concerns with it being called again in the destructor.
        m_renderingRevoker.revoke();
        return;
      }
    }

    // If this is the first time we're rendering a frame, invoke m_onLoadEnd
    if (!m_invokedOnLoadEnd) {
      m_onLoadEnd(!m_nextFrameData);
      m_invokedOnLoadEnd = true;
    }

    // If we failed to render a frame, unsubscribe from the rendering event
    if (!m_nextFrameData) {
      m_renderingRevoker.revoke();
      return;
    }

    // Copy frame data to WriteableBitmap and set source
    winrt::WriteableBitmap frame{m_canvasWidth, m_canvasHeight};
    memcpy(frame.PixelBuffer().data(), m_nextFrameData, m_canvasWidth * m_canvasHeight * sizeof(uint32_t));
    imageBrush.ImageSource(frame);

    // Update the timestamp for the next frame
    m_currentDuration = m_nextTimestamp - m_currentTimestamp;
    m_currentTimestamp = m_nextTimestamp;
    m_frameStart = winrt::clock::now();

    // Kick off another frame
    m_createNextFrameTask = CreateNextFrameAsync();
  }
}

winrt::IAsyncAction WebPAnimator::CreateNextFrameAsync() {
  // Switch to thread pool thread
  co_await winrt::resume_background();

  // Render next frame to internal animation buffer
  if (!WebPAnimDecoderGetNext(m_animDecoder, &m_nextFrameData, &m_nextTimestamp)) {
    // Set the frame data pointer to null so the rendering loop will know decoding failed
    m_nextFrameData = nullptr;
  }
}

} // namespace react::uwp

#endif
