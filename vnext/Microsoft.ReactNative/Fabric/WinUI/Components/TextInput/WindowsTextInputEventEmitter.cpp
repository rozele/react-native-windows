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

static jsi::Value keyPressMetricsPayload(jsi::Runtime &runtime, WindowsKeyPressMetrics const &keyPressMetrics) {
  auto payload = jsi::Object(runtime);
  payload.setProperty(runtime, "text", jsi::String::createFromUtf8(runtime, keyPressMetrics.text));
  payload.setProperty(runtime, "eventCount", keyPressMetrics.eventCount);
  return payload;
};

void WindowsTextInputEventEmitter::onChange(WindowsTextInputMetrics const &textInputMetrics) const {
  dispatchEvent("change", [textInputMetrics](jsi::Runtime &runtime) {
    return textInputMetricsPayload(runtime, textInputMetrics);
  });
}

void WindowsTextInputEventEmitter::onSelectionChange(WindowsTextInputMetrics const &textInputMetrics) const {
  dispatchEvent("textInputSelectionChange", [textInputMetrics](jsi::Runtime &runtime) {
    return textInputMetricsPayload(runtime, textInputMetrics);
  });
}

void WindowsTextInputEventEmitter::onSubmitEditing(WindowsTextInputMetrics const &textInputMetrics) const {
  dispatchEvent("textInputSubmitEditing", [textInputMetrics](jsi::Runtime &runtime) {
    return textInputMetricsPayload(runtime, textInputMetrics);
  });
}

void WindowsTextInputEventEmitter::onKeyPress(WindowsKeyPressMetrics const &keyPressMetrics) const {
  dispatchEvent("textInputKeyPress", [keyPressMetrics](jsi::Runtime &runtime) {
    return keyPressMetricsPayload(runtime, keyPressMetrics);
  });
}

} // namespace facebook::react
