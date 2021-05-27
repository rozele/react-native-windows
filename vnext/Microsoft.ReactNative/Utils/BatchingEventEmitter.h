// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "JSValue.h"
#include "ReactHost/React.h"
#include "ReactPropertyBag.h"
#include "winrt/Microsoft.ReactNative.h"

#include <UI.Xaml.Media.h>
#include <deque>
#include <mutex>

namespace react::uwp::implementation {
struct BatchedEvent {
  winrt::hstring eventEmitterName;
  winrt::hstring emitterMethod;
  winrt::hstring eventName;
  int64_t coalescingKey;
  folly::dynamic params;
};
} // namespace winrt::Microsoft::ReactNative::implementation

namespace react::uwp {

//! Emits events from native to JS in queued batches (at most once per-native frame). Events within a batch may be
//! coalesced. The batch is finished at the time the JS thread starts to process it. I.e. it is possible for a batch to
//! last for multiple frames if the JS thread is blocked. This is by-design as it allows our coalescing strategy to
//! account for long operations on the JS thread.
struct BatchingEventEmitter : public std::enable_shared_from_this<BatchingEventEmitter> {
 public:
  BatchingEventEmitter(const std::weak_ptr<IReactInstance> &reactInstance) noexcept;

  //! Dispatches an event from a view manager.
  void DispatchEvent(int64_t tag, winrt::hstring &&eventName, const winrt::Microsoft::ReactNative::JSValueArgWriter &eventDataWriter) noexcept;
  //! Queues an event to be fired.
  void EmitJSEvent(
      winrt::hstring &&eventEmitterName,
      winrt::hstring &&emitterMethod,
      const winrt::Microsoft::ReactNative::JSValueArgWriter &params) noexcept;

  //! Dispatches an event from a view manager. Existing events in the batch with the same name and tag will be removed.
  void DispatchCoalescingEvent(int64_t tag, winrt::hstring &&eventName, const winrt::Microsoft::ReactNative::JSValueArgWriter &eventDataWriter) noexcept;
  //! Queues an event to be fired. Existing events in the batch with the same name and tag will be removed.
  void EmitCoalescingJSEvent(
      winrt::hstring &&eventEmitterName,
      winrt::hstring &&emitterMethod,
      winrt::hstring &&eventName,
      int64_t coalescingKey,
      const winrt::Microsoft::ReactNative::JSValueArgWriter &params) noexcept;

 private:
  size_t GetCoalescingEventKey(const winrt::hstring &eventEmitterName, const winrt::hstring &emitterMethod, const winrt::hstring &eventName);
  void AddOrCoalesceEvent(implementation::BatchedEvent &&event);
  void RegisterFrameCallback() noexcept;
  void OnFrameUI() noexcept;
  void OnFrameJS() noexcept;

  std::weak_ptr<react::uwp::IReactInstance> m_reactInstance;
  std::deque<implementation::BatchedEvent> m_eventQueue;
  std::map<std::tuple<winrt::hstring, winrt::hstring, winrt::hstring>, size_t> m_coalescingEventIds;
  std::map<std::tuple<int64_t, size_t>, size_t> m_lastEventIndex;
  std::mutex m_eventQueueMutex;
  xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;
  winrt::Microsoft::ReactNative::IReactDispatcher m_uiDispatcher;
};

} // namespace react::uwp
