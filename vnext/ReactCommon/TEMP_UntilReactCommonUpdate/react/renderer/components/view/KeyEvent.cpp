/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "KeyEvent.h"

namespace facebook::react {

#if RN_DEBUG_STRING_CONVERTIBLE

std::string getDebugName(BaseKeyEvent const & /*keyEvent*/) {
  return "KeyEvent";
}

std::vector<DebugStringConvertibleObject> getDebugProps(
    BaseKeyEvent const &keyEvent,
    DebugStringConvertibleOptions options) {
  return {
      {"code", getDebugDescription(keyEvent.code, options)},
      {"altKey", getDebugDescription(keyEvent.altKey, options)},
      {"ctrlKey", getDebugDescription(keyEvent.ctrlKey, options)},
      {"shiftKey", getDebugDescription(keyEvent.shiftKey, options)},
      {"metaKey", (keyEvent.metaKey, options)},
  };
}

#endif

} // namespace facebook::react
