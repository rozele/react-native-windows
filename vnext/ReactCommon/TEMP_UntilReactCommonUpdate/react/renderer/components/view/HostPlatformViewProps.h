/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#ifdef USE_WINUI_FABRIC
#include <react/renderer/components/view/windows/WindowsViewProps.h>

namespace facebook::react {
using HostPlatformViewProps = WindowsViewProps;
} // namespace facebook::react
#else
#include <react/renderer/core/Props.h>
#include <react/renderer/core/PropsParserContext.h>

namespace facebook::react {
class HostPlatformViewProps {
  HostPlatformViewProps() = default;
  HostPlatformViewProps(
      const PropsParserContext &context,
      const HostPlatformViewProps &sourceProps,
      const RawProps &rawProps,
      bool shouldSetRawProps = true) {}

  void
  setProp(
      const PropsParserContext &context,
      RawPropsPropNameHash hash,
      const char *propName,
      RawValue const &value) {}
};
} // namespace facebook::react
#endif
