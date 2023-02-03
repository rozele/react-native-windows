/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WindowsViewEventEmitter.h"

namespace facebook::react {

#pragma mark - Mouse Events

static jsi::Value touchPayload(jsi::Runtime &runtime, Touch const &touch) {
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

void WindowsViewEventEmitter::onMouseEnter(Touch const &touch) const {
  dispatchEvent(
      "mouseEnter",
      [touch](jsi::Runtime &runtime) { return touchPayload(runtime, touch); },
      EventPriority::AsynchronousBatched);
}

void WindowsViewEventEmitter::onMouseLeave(Touch const &touch) const {
  dispatchEvent(
      "mouseLeave",
      [touch](jsi::Runtime &runtime) { return touchPayload(runtime, touch); },
      EventPriority::AsynchronousBatched);
}

#pragma mark - Keyboard Events

static jsi::Value keyEventPayload(jsi::Runtime &runtime, KeyEvent const &event) {
  auto payload = jsi::Object(runtime);
  payload.setProperty(runtime, "key", jsi::String::createFromUtf8(runtime, event.key));
  payload.setProperty(runtime, "code", jsi::String::createFromUtf8(runtime, event.code));
  payload.setProperty(runtime, "ctrlKey", event.ctrlKey);
  payload.setProperty(runtime, "shiftKey", event.shiftKey);
  payload.setProperty(runtime, "altKey", event.altKey);
  payload.setProperty(runtime, "metaKey", event.metaKey);
  payload.setProperty(runtime, "timestamp", event.timestamp * 1000);
  return payload;
};

void WindowsViewEventEmitter::onKeyDown(KeyEvent const &keyEvent) const {
  dispatchEvent(
      "keyDown",
      [keyEvent](jsi::Runtime &runtime) { return keyEventPayload(runtime, keyEvent); },
      EventPriority::AsynchronousBatched);
}

void WindowsViewEventEmitter::onKeyUp(KeyEvent const &keyEvent) const {
  dispatchEvent(
      "keyUp",
      [keyEvent](jsi::Runtime &runtime) { return keyEventPayload(runtime, keyEvent); },
      EventPriority::AsynchronousBatched);
}

} // namespace facebook::react
