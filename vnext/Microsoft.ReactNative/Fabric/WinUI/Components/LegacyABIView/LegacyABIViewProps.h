// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <folly/dynamic.h>
#include <react/renderer/components/view/ViewProps.h>
#include <react/renderer/core/PropsParserContext.h>
#include <unordered_map>

namespace facebook::react {

class LegacyABIViewProps final : public ViewProps {
 public:
  LegacyABIViewProps() = default;
  LegacyABIViewProps(
      const PropsParserContext &context,
      const LegacyABIViewProps &sourceProps,
      const RawProps &rawProps);

#pragma mark - Props

  folly::dynamic const otherProps;
};

} // namespace facebook::react
