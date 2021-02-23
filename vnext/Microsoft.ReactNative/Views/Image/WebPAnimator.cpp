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
using namespace Windows::System;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
} // namespace winrt

namespace react::uwp {

winrt::IAsyncOperation<bool> WebPAnimator::SetSourceAsync(winrt::IRandomAccessStream inputStream) {
  auto weakThis = weak_from_this();

  // Get the current dispatcher queue.
  winrt::DispatcherQueue dispatcherQueue = winrt::DispatcherQueue::GetForCurrentThread();

  // Switch to thread pool thread
  co_await winrt::resume_background();

  // Copy stream contents to memory buffer
  auto length{static_cast<uint32_t>(inputStream.Size())};
  winrt::DataReader reader{inputStream};
  std::vector<uint8_t> buffer;
  buffer.resize(length);
  co_await reader.LoadAsync(length);
  reader.ReadBytes(buffer);

  // Initialize WebP container
  WebPData webpData;
  webpData.bytes = buffer.data();
  webpData.size = buffer.size();

  // Create demuxer to read WebP metadata
  auto demuxer = WebPDemux(&webpData);
  if (!demuxer) {
    co_return false;
  }

  // Get WebP image metadata
  auto canvasWidth = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH);
  auto canvasHeight = WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT);
  auto loopCount = WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT);
  auto frameCount = WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
  WebPDemuxDelete(demuxer);

  // Create WebPAnimDecoder
  WebPAnimDecoderOptions decOptions;
  if (!WebPAnimDecoderOptionsInit(&decOptions)) {
    co_return false;
  }

  // Create WebPAnimDecoder
  decOptions.color_mode = MODE_bgrA;
  auto animDecoder = WebPAnimDecoderNew(&webpData, &decOptions);
  if (!animDecoder) {
    co_return false;
  }

  // Decode all frames
  std::vector<int> timestamps;
  timestamps.reserve(frameCount);
  std::vector<uint8_t> frameData;
  auto frameSize = static_cast<size_t>(canvasWidth * canvasHeight * sizeof(uint32_t));
  frameData.resize(frameSize * frameCount);
  for (uint32_t i = 0; i < frameCount; ++i) {
    if (!WebPAnimDecoderHasMoreFrames(animDecoder)) {
      co_return false;
    }

    uint8_t *nextFrameData;
    int timestamp;
    if (!WebPAnimDecoderGetNext(animDecoder, &nextFrameData, &timestamp)) {
      co_return false;
    }

    memcpy(frameData.data() + i * frameSize, nextFrameData, frameSize);
    timestamps.push_back(timestamp);
  }

  // Delete WebPAnimDecoder
  WebPAnimDecoderDelete(animDecoder);

  {
    // Update WebP image metadata
    if (auto backgroundStrongThis = weakThis.lock()) {
      backgroundStrongThis->m_canvasWidth = canvasWidth;
      backgroundStrongThis->m_canvasHeight = canvasHeight;
      backgroundStrongThis->m_loopCount = loopCount;
      backgroundStrongThis->m_timestamps = std::move(timestamps);
    }
  }

  // Switch back to foreground thread to create DependencyObjects for frames
  co_await winrt::resume_foreground(dispatcherQueue);

  auto strongThis = weakThis.lock();
  auto strongBrush = m_weakBrush.get();
  if (strongThis && strongBrush) {
    // Create WriteableBitmap images for each frame
    strongThis->m_frames.reserve(frameCount);
    for (uint32_t i = 0; i < frameCount; ++i) {
      winrt::WriteableBitmap bitmap{static_cast<int>(canvasWidth), static_cast<int>(canvasHeight)};
      memcpy(bitmap.PixelBuffer().data(), frameData.data() + i * frameSize, frameSize);
      strongThis->m_frames.push_back(bitmap);
    }
  }

  co_return true;
}

WebPAnimator::~WebPAnimator() {
  // It is safe to call revoke regardless of whether or not it's been set
  m_renderingRevoker.revoke();
}

void WebPAnimator::Start() {
  // Set the first frame of the WebP image
  auto strongBrush = m_weakBrush.get();
  if (strongBrush && m_frames.size() >= 1) {
    strongBrush.ImageSource(m_frames[0]);
  }

  // If there are multiple frames, start the animation
  if (m_frames.size() > 1) {
    // The CompositionTarget rendering event is called once per frame after layout has been computed.
    // This feels like a heavyweight approach, but DispatcherTimer resulted in slow animations.
    m_frameStart = winrt::clock::now();
    m_renderingRevoker = winrt::CompositionTarget::Rendering(winrt::auto_revoke, [weakThis = weak_from_this()](auto &&...) {
      if (auto strongThis = weakThis.lock()) {
        strongThis->DisplayNextFrame();
      }
    });
  }
}

void WebPAnimator::DisplayNextFrame() {
  // If frame duration has not passed, return
  auto previousTimestamp = m_frameIndex > 0 ? m_timestamps[static_cast<size_t>(m_frameIndex - 1)] : 0;
  auto duration = std::chrono::milliseconds(m_timestamps[m_frameIndex] - previousTimestamp);
  if ((winrt::clock::now() - m_frameStart) < duration) {
    return;
  }

  // Unsubscribe if this animator no longer has access to the ImageBrush
  auto strongBrush = m_weakBrush.get();
  if (!strongBrush) {
    m_renderingRevoker.revoke();
    return;
  }

  // Set the ImageSource to the next frame
  strongBrush.ImageSource(m_frames[m_frameIndex++]);
  // Reset the frame start time
  m_frameStart = winrt::clock::now();

  // If we've reached the last frame, check if image should loop and reset
  if (m_frameIndex == m_frames.size()) {
    if (m_loopCount != 0 && ++m_loopIndex == m_loopCount) {
      m_renderingRevoker.revoke();
    } else {
      m_frameIndex = 0;
    }
  }
}

} // namespace react::uwp

#endif
