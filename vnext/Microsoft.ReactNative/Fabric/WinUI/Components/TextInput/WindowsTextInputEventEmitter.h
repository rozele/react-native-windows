// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/attributedstring/AttributedString.h>
#include <react/renderer/components/view/ViewEventEmitter.h>

namespace facebook::react {

class WindowsTextInputMetrics {
 public:
  std::string text;
  AttributedString::Range selectionRange{};
  int eventCount{};
};

class WindowsTextInputEventEmitter : public ViewEventEmitter {
 public:
  using ViewEventEmitter::ViewEventEmitter;

  struct Selection {
    int start;
    int end;
  };

  void onChange(WindowsTextInputMetrics const &textInputMetrics) const;
  void onSelectionChange(WindowsTextInputMetrics const &value) const;
  void onSubmitEditing(WindowsTextInputMetrics const &textInputMetrics) const;
};

} // namespace facebook::react
