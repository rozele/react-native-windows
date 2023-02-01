// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/components/rnwcore/Props.h>
#include <react/renderer/components/text/BaseTextProps.h>
#include <react/renderer/components/view/windows/KeyEvent.h>
#include <react/renderer/core/propsConversions.h>

namespace facebook::react {

struct WindowsTextInputSelectionStruct {
  int start;
  int end;
};

static inline void
fromRawValue(const PropsParserContext &context, const RawValue &value, WindowsTextInputSelectionStruct &result) {
  auto map = (butter::map<std::string, RawValue>)value;

  auto tmp_start = map.find("start");
  if (tmp_start != map.end()) {
    fromRawValue(context, tmp_start->second, result.start);
  }
  auto tmp_end = map.find("end");
  if (tmp_end != map.end()) {
    fromRawValue(context, tmp_end->second, result.end);
  }
}

static inline std::string toString(const WindowsTextInputSelectionStruct &value) {
  return "[Object WindowsTextInputSelectionStruct]";
}

class WindowsTextInputProps final : public ViewProps, public BaseTextProps {
 public:
  WindowsTextInputProps() = default;
  WindowsTextInputProps(
      const PropsParserContext &context,
      const WindowsTextInputProps &sourceProps,
      const RawProps &rawProps);

  bool allowFontScaling{true};
  bool clearTextOnFocus{false};
  bool editable{true};
  int maxLength{0};
  bool multiline{false};
  std::string placeholder{};
  SharedColor placeholderTextColor{};
  bool scrollEnabled{true};
  WindowsTextInputSelectionStruct selection{};
  SharedColor selectionColor{};
  bool selectTextOnFocus{false};
  bool spellCheck{false};
  std::string text{};
  int mostRecentEventCount{0};
  bool secureTextEntry{false};
  std::string keyboardType{};
  bool contextMenuHidden{false};
  bool caretHidden{false};
  std::string autoCapitalize{};
  bool clearTextOnSubmit{false};
  std::vector<facebook::react::HandledKeyEvent> submitKeyEvents{};
  bool autoFocus{false};

  /**
   * Auxiliary information to detect if these props are set or not.
   * See AndroidTextInputComponentDescriptor for usage.
   * TODO T63008435: can these, and this feature, be removed entirely?
   */
  const bool hasPadding{};
  const bool hasPaddingHorizontal{};
  const bool hasPaddingVertical{};
  const bool hasPaddingLeft{};
  const bool hasPaddingTop{};
  const bool hasPaddingRight{};
  const bool hasPaddingBottom{};
  const bool hasPaddingStart{};
  const bool hasPaddingEnd{};
};

} // namespace facebook::react
