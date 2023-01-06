/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ViewEventEmitter.h"

namespace facebook {
namespace react {

#pragma mark - Accessibility

void ViewEventEmitter::onAccessibilityAction(std::string const &name) const {
  dispatchEvent("accessibilityAction", [name](jsi::Runtime &runtime) {
    auto payload = jsi::Object(runtime);
    payload.setProperty(runtime, "actionName", name);
    return payload;
  });
}

void ViewEventEmitter::onAccessibilityTap() const {
  dispatchEvent("accessibilityTap");
}

void ViewEventEmitter::onAccessibilityMagicTap() const {
  dispatchEvent("magicTap");
}

void ViewEventEmitter::onAccessibilityEscape() const {
  dispatchEvent("accessibilityEscape");
}

#pragma mark - Layout

void ViewEventEmitter::onLayout(const LayoutMetrics &layoutMetrics) const {
  // A copy of a shared pointer (`layoutEventState_`) establishes shared
  // ownership that will be captured by lambda.
  auto layoutEventState = layoutEventState_;

  // Dispatched `frame` values to JavaScript thread are throttled here.
  // Basic ideas:
  //  - Scheduling a lambda with some value that already was dispatched, does
  //    nothing.
  //  - If some lambda is already in flight, we don't schedule another;
  //  - When a lambda is being executed on the JavaScript thread, the *most
  //    recent* `frame` value is used (not the value that was current at the
  //    moment of scheduling the lambda).
  //
  // This implies the following caveats:
  //  - Some events can be skipped;
  //  - When values change rapidly, even events with different values
  //    can be skipped (only the very last will be delivered).
  //  - Ordering is preserved.

  {
    std::lock_guard<std::mutex> guard(layoutEventState->mutex);

    // If a *particular* `frame` was already dispatched to the JavaScript side,
    // no other work is required.
    if (layoutEventState->frame == layoutMetrics.frame &&
        layoutEventState->wasDispatched) {
      return;
    }

    // If the *particular* `frame` was not already dispatched *or*
    // some *other* `frame` was dispatched before,
    // we need to schedule the dispatching.
    layoutEventState->wasDispatched = false;
    layoutEventState->frame = layoutMetrics.frame;

    // Something is already in flight, dispatching another event is not
    // required.
    if (layoutEventState->isDispatching) {
      return;
    }

    layoutEventState->isDispatching = true;
  }

  dispatchEvent(
      "layout",
      [layoutEventState](jsi::Runtime &runtime) {
        auto frame = Rect{};

        {
          std::lock_guard<std::mutex> guard(layoutEventState->mutex);

          layoutEventState->isDispatching = false;

          // If some *particular* `frame` was already dispatched before,
          // and since then there were no other new values of the `frame`
          // observed, do nothing.
          if (layoutEventState->wasDispatched) {
            return jsi::Value::null();
          }

          frame = layoutEventState->frame;

          // If some *particular* `frame` was *not* already dispatched before,
          // it's time to dispatch it and mark as dispatched.
          layoutEventState->wasDispatched = true;
        }

        auto layout = jsi::Object(runtime);
        layout.setProperty(runtime, "x", frame.origin.x);
        layout.setProperty(runtime, "y", frame.origin.y);
        layout.setProperty(runtime, "width", frame.size.width);
        layout.setProperty(runtime, "height", frame.size.height);
        auto payload = jsi::Object(runtime);
        payload.setProperty(runtime, "layout", std::move(layout));
        return jsi::Value(std::move(payload));
      },
      EventPriority::AsynchronousUnbatched);
}

#ifdef USE_WINUI_FABRIC
#pragma mark - Mouse Events

static jsi::Value touchPayload(
    jsi::Runtime &runtime,
    Touch const &touch) {
  auto payload = jsi::Object(runtime);
  payload.setProperty(runtime, "locationX", touch.offsetPoint.x);
  payload.setProperty(runtime, "locationY", touch.offsetPoint.y);
  payload.setProperty(runtime, "pageX", touch.pagePoint.x);
  payload.setProperty(runtime, "pageY", touch.pagePoint.y);
  payload.setProperty(runtime, "screenX", touch.screenPoint.x);
  payload.setProperty(runtime, "screenY", touch.screenPoint.y);
  payload.setProperty(runtime, "identifier", touch.identifier);
  payload.setProperty(runtime, "target", touch.target);
  payload.setProperty(runtime, "timestamp", touch.timestamp * 1000);
  payload.setProperty(runtime, "force", touch.force);
  payload.setProperty(runtime, "button", touch.button);
  payload.setProperty(runtime, "altKey", touch.altKey);
  payload.setProperty(runtime, "ctrlKey", touch.ctrlKey);
  payload.setProperty(runtime, "shiftKey", touch.shiftKey);
  return payload;
};

void ViewEventEmitter::onMouseEnter(const Touch &touch) const {
  dispatchEvent(
      "mouseEnter",
      [touch](jsi::Runtime &runtime) {
        return touchPayload(runtime, touch);
      },
      EventPriority::AsynchronousBatched);
}

void ViewEventEmitter::onMouseLeave(const Touch &touch) const {
  dispatchEvent(
      "mouseLeave",
      [touch](jsi::Runtime &runtime) {
        return touchPayload(runtime, touch);
      },
      EventPriority::AsynchronousBatched);
}

#endif
} // namespace react
} // namespace facebook
