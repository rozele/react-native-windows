// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/attributedstring/AttributedString.h>
#include <react/renderer/components/view/ViewEventEmitter.h>

namespace facebook::react {

class WindowsTextInputMetrics {
 public:
  std::string text;
  AttributedString::Range selectionRange;
  // ScrollView-like metrics
  Size contentSize;
  int eventCount;
  Size layoutMeasurement;
  float zoomScale;
};

class WindowsTextInputEventEmitter : public ViewEventEmitter {
 public:
  using ViewEventEmitter::ViewEventEmitter;

  struct Selection {
    int start;
    int end;
  };

  struct OnChange {
    int eventCount;
    int target;
    std::string text;
  };

  struct OnSelectionChange {
    Selection selection;
  };

  void onChange(OnChange value) const;
  void onSelectionChange(const OnSelectionChange &value) const;
  void onSubmitEditing(WindowsTextInputMetrics const &textInputMetrics) const;
};

} // namespace facebook::react
