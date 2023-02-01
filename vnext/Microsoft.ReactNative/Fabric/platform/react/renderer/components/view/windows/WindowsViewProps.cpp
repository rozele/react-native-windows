/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WindowsViewProps.h"

#include <react/renderer/components/view/windows/conversions.h>
#include <react/renderer/core/CoreFeatures.h>
#include <react/renderer/core/propsConversions.h>

namespace facebook::react {

WindowsViewProps::WindowsViewProps(
    const PropsParserContext &context,
    const WindowsViewProps &sourceProps,
    const RawProps &rawProps,
    bool shouldSetRawProps)
    : windowsEvents(
          CoreFeatures::enablePropIteratorSetter ? sourceProps.windowsEvents
                                                 : convertRawProp(context, rawProps, sourceProps.windowsEvents, {})),
      focusable(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.focusable
              : convertRawProp(context, rawProps, "focusable", sourceProps.focusable, {})),
      enableFocusRing(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.enableFocusRing
              : convertRawProp(context, rawProps, "enableFocusRing", sourceProps.enableFocusRing, true)),
      overflowAnchor(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.overflowAnchor
              : convertRawProp(context, rawProps, "overflowAnchor", sourceProps.overflowAnchor, {})),
      tooltip(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.tooltip
              : convertRawProp(context, rawProps, "tooltip", sourceProps.tooltip, {})),
      keyDownEvents(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.keyDownEvents
              : convertRawProp(context, rawProps, "keyDownEvents", sourceProps.keyDownEvents, {})),
      keyUpEvents(
          CoreFeatures::enablePropIteratorSetter
              ? sourceProps.keyUpEvents
              : convertRawProp(context, rawProps, "keyUpEvents", sourceProps.keyUpEvents, {})){};

#define VIEW_EVENT_CASE_WINDOWS(eventType, eventString) \
  case CONSTEXPR_RAW_PROPS_KEY_HASH(eventString): {     \
    WindowsViewEvents defaultViewEvents{};              \
    bool res = defaultViewEvents[eventType];            \
    if (value.hasValue()) {                             \
      fromRawValue(context, value, res);                \
    }                                                   \
    windowsEvents[eventType] = res;                     \
    return;                                             \
  }

void WindowsViewProps::setProp(
    const PropsParserContext &context,
    RawPropsPropNameHash hash,
    const char *propName,
    RawValue const &value) {
  switch (hash) {
    VIEW_EVENT_CASE_WINDOWS(WindowsViewEvents::Offset::MouseEnter, "onMouseEnter");
    VIEW_EVENT_CASE_WINDOWS(WindowsViewEvents::Offset::MouseLeave, "onMouseLeave");
    VIEW_EVENT_CASE_WINDOWS(WindowsViewEvents::Offset::KeyDown, "onKeyDown");
    VIEW_EVENT_CASE_WINDOWS(WindowsViewEvents::Offset::KeyUp, "onKeyUp");
    RAW_SET_PROP_SWITCH_CASE_BASIC(focusable, false);
    RAW_SET_PROP_SWITCH_CASE_BASIC(enableFocusRing, true);
    RAW_SET_PROP_SWITCH_CASE_BASIC(overflowAnchor, {});
    RAW_SET_PROP_SWITCH_CASE_BASIC(tooltip, {});
    RAW_SET_PROP_SWITCH_CASE_BASIC(keyDownEvents, {});
    RAW_SET_PROP_SWITCH_CASE_BASIC(keyUpEvents, {});
  }
}

} // namespace facebook::react
