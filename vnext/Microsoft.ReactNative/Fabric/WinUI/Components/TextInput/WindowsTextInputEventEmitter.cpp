// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "WindowsTextInputEventEmitter.h"

namespace facebook::react {

static jsi::Value textInputMetricsPayload(jsi::Runtime &runtime, WindowsTextInputMetrics const &textInputMetrics) {
  auto payload = jsi::Object(runtime);
  payload.setProperty(runtime, "text", jsi::String::createFromUtf8(runtime, textInputMetrics.text));
  payload.setProperty(runtime, "eventCount", textInputMetrics.eventCount);

  {
    auto selection = jsi::Object(runtime);
    selection.setProperty(runtime, "start", textInputMetrics.selectionRange.location);
    selection.setProperty(
        runtime, "end", textInputMetrics.selectionRange.location + textInputMetrics.selectionRange.length);
    payload.setProperty(runtime, "selection", selection);
  }

  return payload;
};

void WindowsTextInputEventEmitter::onChange(OnChange event) const {
  dispatchEvent("change", [event = std::move(event)](jsi::Runtime &runtime) {
    auto payload = jsi::Object(runtime);
    payload.setProperty(runtime, "eventCount", event.eventCount);
    payload.setProperty(runtime, "target", event.target);
    payload.setProperty(runtime, "text", event.text);
    return payload;
  });
}

void WindowsTextInputEventEmitter::onSelectionChange(const OnSelectionChange &event) const {
  dispatchEvent("textInputSelectionChange", [event = std::move(event)](jsi::Runtime &runtime) {
    auto payload = jsi::Object(runtime);
    auto selection = jsi::Object(runtime);
    selection.setProperty(runtime, "start", event.selection.start);
    selection.setProperty(runtime, "end", event.selection.end);
    payload.setProperty(runtime, "selection", selection);
    return payload;
  });
}

void WindowsTextInputEventEmitter::onSubmitEditing(WindowsTextInputMetrics const &textInputMetrics) const {
  dispatchEvent("textInputSubmitEditing", [textInputMetrics](jsi::Runtime &runtime) {
    return textInputMetricsPayload(runtime, textInputMetrics);
  });
}

} // namespace facebook::react
