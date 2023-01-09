/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsynchronousEventBeat.h"

#include <react/debug/react_native_assert.h>

namespace facebook::react {

AsynchronousEventBeat::AsynchronousEventBeat(
    RunLoopObserver::Unique uiRunLoopObserver,
    RuntimeExecutor runtimeExecutor)
    : EventBeat({}),
      uiRunLoopObserver_(std::move(uiRunLoopObserver)),
      runtimeExecutor_(std::move(runtimeExecutor)) {
  uiRunLoopObserver_->setDelegate(this);
#ifndef USE_WINUI_FABRIC
  uiRunLoopObserver_->enable();
#endif
}

void AsynchronousEventBeat::activityDidChange(
    RunLoopObserver::Delegate const *delegate,
    RunLoopObserver::Activity /*activity*/) const noexcept {
  react_native_assert(delegate == this);
  induce();
}

#ifdef USE_WINUI_FABRIC
void AsynchronousEventBeat::request() const {
  // This is a no-op if the run loop has already been enabled.
  uiRunLoopObserver_->enable();
  EventBeat::request();
}

#endif

void AsynchronousEventBeat::induce() const {
#ifdef USE_WINUI_FABRIC
  const bool isRequested = isRequested_;
  isRequested_ = false;
  uiRunLoopObserver_->disable();

  if (!isRequested || isBeatCallbackScheduled_) {
    return;
  }
#else
  if (!isRequested_ || isBeatCallbackScheduled_) {
    return;
  }

  isRequested_ = false;
#endif

  // Here we know that `this` object exists because the caller has a strong
  // pointer to `owner`. To ensure the object will exist inside
  // `runtimeExecutor_` callback, we need to copy the  pointer there.
  auto weakOwner = uiRunLoopObserver_->getOwner();

  isBeatCallbackScheduled_ = true;

  runtimeExecutor_([this, weakOwner](jsi::Runtime &runtime) {
    isBeatCallbackScheduled_ = false;

    auto owner = weakOwner.lock();
    if (!owner) {
      return;
    }

    if (beatCallback_) {
      beatCallback_(runtime);
    }
  });
}
} // namespace facebook::react
