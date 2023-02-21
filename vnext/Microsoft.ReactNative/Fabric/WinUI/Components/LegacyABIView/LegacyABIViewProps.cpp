// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "LegacyABIViewProps.h"
#include <react/renderer/core/DynamicPropsUtilities.h>

namespace facebook::react {

LegacyABIViewProps::LegacyABIViewProps(
    const PropsParserContext &context,
    const LegacyABIViewProps &sourceProps,
    const RawProps &rawProps)
    : ViewProps(context, sourceProps, rawProps),
      otherProps(mergeDynamicProps(sourceProps.otherProps, (folly::dynamic)rawProps)) {}

} // namespace facebook::react
