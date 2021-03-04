// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "BatchingEventEmitter.h"
#include "DynamicWriter.h"
#include "JSValueWriter.h"

#include <ReactHost/UwpReactInstanceProxy.h>
#include <ReactHost/ReactInstanceWin.h>

using namespace winrt::Microsoft::ReactNative;

namespace react::uwp {

IReactPropertyBag GetProperties(const std::weak_ptr<react::uwp::IReactInstance> reactInstance) {
  if (const auto instance = reactInstance.lock()) {
    if (const auto reactInstanceWin = static_cast<Mso::React::ReactInstanceWin*>(static_cast<react::uwp::UwpReactInstanceProxy*>(instance.get())->GetReactInstance().Get())) {
      return reactInstanceWin->Options().Properties;
    }
  }

  return nullptr;
}

BatchingEventEmitter::BatchingEventEmitter(const std::weak_ptr<react::uwp::IReactInstance> &reactInstance) noexcept
    : m_reactInstance(reactInstance) {
  if (const auto properties = GetProperties(m_reactInstance)) {
    m_uiDispatcher = properties.Get(ReactDispatcherHelper::UIDispatcherProperty()).as<IReactDispatcher>();
  }
}

void BatchingEventEmitter::DispatchEvent(
    int64_t tag,
    winrt::hstring &&eventName,
    const JSValueArgWriter &eventDataWriter) noexcept {
  return EmitJSEvent(
      L"RCTEventEmitter",
      L"receiveEvent",
      [tag, eventName = std::move(eventName), eventDataWriter](const IJSValueWriter &paramsWriter) mutable {
        paramsWriter.WriteArrayBegin();
        WriteValue(paramsWriter, tag);
        WriteValue(paramsWriter, std::move(eventName));
        eventDataWriter(paramsWriter);
        paramsWriter.WriteArrayEnd();
      });
}

void BatchingEventEmitter::EmitJSEvent(
    winrt::hstring &&eventEmitterName,
    winrt::hstring &&emitterMethod,
    const JSValueArgWriter &eventDataWriter) noexcept {
  if (!m_uiDispatcher)
    return;

  VerifyElseCrash(m_uiDispatcher.HasThreadAccess());

  implementation::BatchedEvent newEvent{
      std::move(eventEmitterName), std::move(emitterMethod), L"", 0, DynamicWriter::ToDynamic(eventDataWriter)};
  bool isFirstEventInBatch = false;

  {
    std::scoped_lock lock(m_eventQueueMutex);

    isFirstEventInBatch = m_eventQueue.size() == 0;
    m_eventQueue.push_back(std::move(newEvent));
  }

  if (isFirstEventInBatch) {
    RegisterFrameCallback();
  }
}

void BatchingEventEmitter::DispatchCoalescingEvent(
    int64_t tag,
    winrt::hstring &&eventName,
    const JSValueArgWriter &eventDataWriter) noexcept {
  EmitCoalescingJSEvent(
      L"RCTEventEmitter",
      L"receiveEvent",
      std::move(eventName),
      tag,
      [tag, eventName = std::move(eventName), &eventDataWriter](const IJSValueWriter &paramsWriter) {
        paramsWriter.WriteArrayBegin();
        WriteValue(paramsWriter, tag);
        WriteValue(paramsWriter, std::move(eventName));
        eventDataWriter(paramsWriter);
        paramsWriter.WriteArrayEnd();
      });
}

void BatchingEventEmitter::EmitCoalescingJSEvent(
    winrt::hstring &&eventEmitterName,
    winrt::hstring &&emitterMethod,
    winrt::hstring &&eventName,
    int64_t coalescingKey,
    const JSValueArgWriter &params) noexcept {
  if (!m_uiDispatcher)
    return;

  VerifyElseCrash(m_uiDispatcher.HasThreadAccess());

  implementation::BatchedEvent newEvent{
      std::move(eventEmitterName),
      std::move(emitterMethod),
      std::move(eventName),
      coalescingKey,
      DynamicWriter::ToDynamic(params)};
  bool isFirstEventInBatch = false;

  {
    std::scoped_lock lock(m_eventQueueMutex);

    isFirstEventInBatch = m_eventQueue.size() == 0;

    auto endIter = std::remove_if(m_eventQueue.begin(), m_eventQueue.end(), [&](const auto &evt) noexcept {
      return evt.eventEmitterName == newEvent.eventEmitterName && evt.emitterMethod == newEvent.emitterMethod &&
          evt.eventName == newEvent.eventName && evt.coalescingKey == newEvent.coalescingKey;
    });

    m_eventQueue.erase(endIter, m_eventQueue.end());
    m_eventQueue.push_back(std::move(newEvent));
  }

  if (isFirstEventInBatch) {
    RegisterFrameCallback();
  }
}

void BatchingEventEmitter::RegisterFrameCallback() noexcept {
  VerifyElseCrash(!m_renderingRevoker);

  m_renderingRevoker = xaml::Media::CompositionTarget::Rendering(
      winrt::auto_revoke, [weakThis{weak_from_this()}](auto const &, auto const &) {
        if (auto strongThis = weakThis.lock()) {
          strongThis->OnFrameUI();
        }
      });
}

void BatchingEventEmitter::OnFrameUI() noexcept {
  if (const auto properties = GetProperties(m_reactInstance)) {
    auto jsDispatcher = properties.Get(ReactDispatcherHelper::JSDispatcherProperty()).as<IReactDispatcher>();

    jsDispatcher.Post([weakThis{weak_from_this()}]() noexcept {
      if (auto strongThis = weakThis.lock()) {
        strongThis->OnFrameJS();
      }
    });

    // Don't leave the callback continuously registered as it can waste power.
    // See https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.media.compositiontarget.rendering?view=winrt-19041
    m_renderingRevoker.revoke();
  }
}

void BatchingEventEmitter::OnFrameJS() noexcept {
  if (const auto reactInstance = m_reactInstance.lock()) {
    std::deque<implementation::BatchedEvent> currentBatch;

    {
      std::scoped_lock lock(m_eventQueueMutex);
      currentBatch.swap(m_eventQueue);
    }

    while (!currentBatch.empty()) {
      auto &evt = currentBatch.front();
      reactInstance->CallJsFunction(
          winrt::to_string(evt.eventEmitterName), winrt::to_string(evt.emitterMethod), std::move(evt.params));
      currentBatch.pop_front();
    }
  }
}

} // namespace react::uwp
