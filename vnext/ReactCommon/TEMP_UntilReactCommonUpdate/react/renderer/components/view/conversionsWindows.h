/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <butter/map.h>
#include <folly/Conv.h>
#include <react/renderer/core/PropsParserContext.h>
#include <react/renderer/components/View/KeyEvent.h>

namespace facebook::react {

inline void fromRawValue(
    const PropsParserContext &context,
    const RawValue &value,
    HandledKeyEvent &result) {
  auto map = static_cast<butter::map<std::string, RawValue>>(value);
  for (const auto &pair : map) {
    if (pair.first == "code") {
      result.code = static_cast<std::string>(pair.second);
    } else if (pair.first == "handledEventPhase") {
      const auto handledEventPhase = static_cast<int32_t>(pair.second);
      result.handledEventPhase = static_cast<HandledEventPhase>(handledEventPhase);
    } else if (pair.first == "altKey") {
      result.altKey = static_cast<bool>(pair.second);
    } else if (pair.first == "ctrlKey") {
      result.ctrlKey = static_cast<bool>(pair.second);
    } else if (pair.first == "shiftKey") {
      result.shiftKey = static_cast<bool>(pair.second);
    } else if (pair.first == "metaKey") {
      result.metaKey = static_cast<bool>(pair.second);
    }
  }
}

} // namespace facebook::react
