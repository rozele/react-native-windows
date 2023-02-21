// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/components/view/ConcreteViewShadowNode.h>
#include "LegacyABIViewEventEmitter.h"
#include "LegacyABIViewProps.h"
#include "LegacyABIViewState.h"

namespace facebook::react {

extern const char LegacyABIViewComponentName[];

class LegacyABIViewShadowNode : public ConcreteViewShadowNode<
                                    LegacyABIViewComponentName,
                                    LegacyABIViewProps,
                                    LegacyABIViewEventEmitter,
                                    LegacyABIViewState> {
 public:
  using ConcreteViewShadowNode::ConcreteViewShadowNode;
};

} // namespace facebook::react
