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
  auto length{static_cast<uint32_t>(inputStream.Size())};
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

  m_canvasWidth = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH);
  m_canvasHeight = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT);
  m_frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
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
  if (!m_animDecoder) {
    m_onLoadEnd(false);
    co_return;
  }

  m_onLoadEnd(true);

  if (m_frameCount == 1) {
    // Not animated
    if (auto strongBrush = m_weakBrush.get()) {
      co_await CreateNextFrameAsync();

      std::lock_guard<std::mutex> lock{m_disposeMutex};

      if (m_nextFrameState == WebPFrameState::Ready) {
        // Copy frame data to WriteableBitmap and set source
        winrt::WriteableBitmap frame{m_canvasWidth, m_canvasHeight};
        memcpy(frame.PixelBuffer().data(), m_nextFrameData, m_canvasWidth * m_canvasHeight * sizeof(uint32_t));
        strongBrush.ImageSource(frame);
      }
    }
  } else if (m_frameCount > 1) {
    // The CompositionTarget rendering event is called once per frame after layout has been computed.
    // This feels like a heavyweight approach, but DispatcherTimer resulted in slow animations.
    m_frameStart = winrt::clock::now();
    CreateNextFrameAsync();
    m_renderingRevoker =
        winrt::CompositionTarget::Rendering(winrt::auto_revoke, {this, &WebPAnimator::DisplayNextFrame});
  }
}

WebPAnimator::~WebPAnimator() {
  if (m_frameCount > 1) {
    m_renderingRevoker.revoke();
  }

  std::lock_guard<std::mutex> lock{m_disposeMutex};

  m_isDisposed = true;

  // This could potentially be released earlier for finite loop animations.
  WebPAnimDecoderDelete(m_animDecoder);
}

void WebPAnimator::DisplayNextFrame(winrt::IInspectable const &sender, winrt::IInspectable const &args) {
  // Unsubscribe if we failed to decode a frame
  if (m_nextFrameState == WebPFrameState::Failed) {
    m_renderingRevoker.revoke();
    return;
  }

  // If the next frame is not ready, return 
  if (m_nextFrameState != WebPFrameState::Ready) {
    return;
  }

  // If the current frame duration has not passed, return
  auto frameDuration = winrt::clock::now() - m_frameStart;
  if (frameDuration < std::chrono::milliseconds(m_currentDurationMs)) {
    return;
  }

  // Unsubscribe if we no longer have a valid reference to the image brush.
  auto strongBrush = m_weakBrush.get();
  if (!strongBrush) {
    m_renderingRevoker.revoke();
    return;
  }

  std::lock_guard<std::mutex> lock{m_disposeMutex};

  // Do not attempt to render the frame if the WebPAnimDecoder was deleted
  if (m_isDisposed) {
    m_renderingRevoker.revoke();
    return;
  }

  // Copy frame data to WriteableBitmap and set source
  winrt::WriteableBitmap frame{m_canvasWidth, m_canvasHeight};
  memcpy(frame.PixelBuffer().data(), m_nextFrameData, m_canvasWidth * m_canvasHeight * sizeof(uint32_t));
  strongBrush.ImageSource(frame);

  // Update the timestamp for the next frame
  m_currentDurationMs = m_nextTimestampMs - m_currentTimestampMs;
  m_currentTimestampMs = m_nextTimestampMs;
  m_frameStart = winrt::clock::now();

  // Check if we've reached the end of the loop
  if (!WebPAnimDecoderHasMoreFrames(m_animDecoder)) {
    if (m_loopCount == 0 || ++m_currentLoopIndex < m_loopCount) {
      // Reset the WebPAnimationDecoder to the first frame
      WebPAnimDecoderReset(m_animDecoder);
      m_currentTimestampMs = 0;
    } else {
      // Unsubscribe from the rendering event, this call is idempotent so
      // there are no concerns with it being called again in the destructor.
      m_renderingRevoker.revoke();
      return;
    }
  }

  // Kick off another frame decoding background task
  CreateNextFrameAsync();
}

winrt::IAsyncAction WebPAnimator::CreateNextFrameAsync() {
  // Must set before switching to background thread
  m_nextFrameState = WebPFrameState::Started;

  // Switch to thread pool thread
  co_await winrt::resume_background();

  std::lock_guard<std::mutex> lock{m_disposeMutex};

  // Do not attempt to decode the next frame if the WebPAnimDecoder is deleted.
  if (m_isDisposed) {
    m_nextFrameState = WebPFrameState::Canceled;
    co_return;
  }

  // Render next frame to internal animation buffer
  if (!WebPAnimDecoderGetNext(m_animDecoder, &m_nextFrameData, &m_nextTimestampMs)) {
    // Set the frame data pointer to null so the rendering loop will know decoding failed
    m_nextFrameState = WebPFrameState::Failed;
    co_return;
  }

  m_nextFrameState = WebPFrameState::Ready;
}

} // namespace react::uwp

#endif
