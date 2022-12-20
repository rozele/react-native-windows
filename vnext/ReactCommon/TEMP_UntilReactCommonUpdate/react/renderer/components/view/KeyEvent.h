/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <react/renderer/core/ReactPrimitives.h>
#include <react/renderer/debug/DebugStringConvertible.h>
#include <react/renderer/graphics/Float.h>

namespace facebook {
namespace react {

/*
 * Describes an individual key event.
 */
struct BaseKeyEvent {
  /**
   * The code for the event aligned to https://www.w3.org/TR/uievents-code/.
   */
  std::string code{};

  /*
   * A flag indicating if the alt key is pressed.
   */
  bool altKey{false};

  /*
   * A flag indicating if the control key is pressed.
   */
  bool ctrlKey{false};

  /*
   * A flag indicating if the shift key is pressed.
   */
  bool shiftKey{false};

  /*
   * A flag indicating if the meta key is pressed.
   */
  bool metaKey{false};
};

/**
 * Event phase indicator matching the EventPhase in React.
 * EventPhase includes None, Capturing, AtTarget, Bubbling.
 */
enum class HandledEventPhase { Capturing = 1, Bubbling = 3 };

/**
 * Describes a handled key event declaration.
 */
struct HandledKeyEvent : BaseKeyEvent {
  /**
   * The phase at which the event should be marked as handled.
   */
  HandledEventPhase handledEventPhase{HandledEventPhase::Bubbling};
};

struct KeyEvent : BaseKeyEvent {
  /**
   * The key for the event aligned to https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key/Key_Values.
   */
  std::string key{};

  /**
   * The native timestamp for the event.
   */
  Float timestamp{};
};

#if RN_DEBUG_STRING_CONVERTIBLE

// TODO(): Add debug utilities for HandledKeyEvent and KeyEvent
std::string getDebugName(BaseKeyEvent const &keyEvent);
std::vector<DebugStringConvertibleObject> getDebugProps(
    BaseKeyEvent const &keyEvent,
    DebugStringConvertibleOptions options);

#endif



} // namespace react
} // namespace facebook
