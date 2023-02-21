// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <react/renderer/core/ConcreteComponentDescriptor.h>
#include <winrt/Microsoft.ReactNative.h>
#include "LegacyABIViewShadowNode.h"

namespace facebook::react {

/*
 * Descriptor for IViewManager components.
 */
class LegacyABIViewComponentDescriptor final : public ConcreteComponentDescriptor<LegacyABIViewShadowNode> {
 public:
  using ConcreteComponentDescriptor::ConcreteComponentDescriptor;

  LegacyABIViewComponentDescriptor(ComponentDescriptorParameters const &parameters);

  /*
   * Returns `name` and `handle` based on a `flavor`, not on static data from
   * `LegacyABIViewComponentDescriptor`.
   */
  ComponentHandle getComponentHandle() const override;
  ComponentName getComponentName() const override;
};

} // namespace facebook::react
