/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <react/renderer/components/view/windows/KeyEvent.h>
#include <react/renderer/components/view/windows/primitives.h>
#include <react/renderer/core/Props.h>
#include <react/renderer/core/PropsParserContext.h>

namespace facebook::react {

class WindowsViewProps {
 public:
  WindowsViewProps() = default;
  WindowsViewProps(
      const PropsParserContext &context,
      const WindowsViewProps &sourceProps,
      const RawProps &rawProps,
      bool shouldSetRawProps = true);

  void
  setProp(const PropsParserContext &context, RawPropsPropNameHash hash, const char *propName, RawValue const &value);

  WindowsViewEvents windowsEvents{};

  bool focusable{false};
  bool enableFocusRing{true};
  std::optional<std::string> overflowAnchor{};
  std::optional<std::string> tooltip{};

  std::vector<HandledKeyEvent> keyDownEvents{};
  std::vector<HandledKeyEvent> keyUpEvents{};
};

} // namespace facebook::react
