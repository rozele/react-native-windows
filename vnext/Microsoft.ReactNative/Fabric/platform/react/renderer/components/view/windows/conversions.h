/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <butter/map.h>
#include <folly/Conv.h>
#include <react/renderer/components/view/windows/KeyEvent.h>
#include <react/renderer/components/view/windows/primitives.h>
#include <react/renderer/core/PropsParserContext.h>
#include <react/renderer/core/propsConversions.h>

namespace facebook::react {

// This can be deleted when non-iterator ViewProp parsing is deleted
static inline WindowsViewEvents convertRawProp(
    const PropsParserContext &context,
    const RawProps &rawProps,
    const WindowsViewEvents &sourceValue,
    const WindowsViewEvents &defaultValue) {
  WindowsViewEvents result{};
  using Offset = WindowsViewEvents::Offset;

  // Mouse Events
  result[Offset::MouseEnter] = convertRawProp(
      context, rawProps, "onMouseEnter", sourceValue[Offset::MouseEnter], defaultValue[Offset::MouseEnter]);
  result[Offset::MouseLeave] = convertRawProp(
      context, rawProps, "onMouseLeave", sourceValue[Offset::MouseLeave], defaultValue[Offset::MouseLeave]);

  // Key Events
  result[Offset::KeyDown] =
      convertRawProp(context, rawProps, "onKeyDown", sourceValue[Offset::KeyDown], defaultValue[Offset::KeyDown]);
  result[Offset::KeyUp] =
      convertRawProp(context, rawProps, "onKeyUp", sourceValue[Offset::KeyUp], defaultValue[Offset::KeyUp]);

  return result;
}

inline void fromRawValue(const PropsParserContext &context, const RawValue &value, HandledKeyEvent &result) {
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
