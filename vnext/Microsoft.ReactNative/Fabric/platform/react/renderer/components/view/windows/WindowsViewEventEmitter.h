/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <react/renderer/components/view/TouchEventEmitter.h>
#include <react/renderer/components/view/windows/KeyEvent.h>

namespace facebook::react {

class WindowsViewEventEmitter : public TouchEventEmitter {
 public:
  using TouchEventEmitter::TouchEventEmitter;
#pragma mark - Mouse Events

  void onMouseEnter(Touch const &touch) const;
  void onMouseLeave(Touch const &touch) const;

#pragma mark - Keyboard Events

  void onKeyDown(KeyEvent const &keyEvent) const;
  void onKeyUp(KeyEvent const &keyEvent) const;

#pragma mark - Focus Events

  void onFocus() const;
  void onBlur() const;
};

} // namespace facebook::react
