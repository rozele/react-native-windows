// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "LegacyABIViewComponentDescriptor.h"
#include <Fabric/WinUI/FabricUIManagerModule.h>

namespace facebook::react {

LegacyABIViewComponentDescriptor::LegacyABIViewComponentDescriptor(ComponentDescriptorParameters const &parameters)
    : ConcreteComponentDescriptor(parameters) {}

ComponentHandle LegacyABIViewComponentDescriptor::getComponentHandle() const {
  return reinterpret_cast<ComponentHandle>(getComponentName());
}

ComponentName LegacyABIViewComponentDescriptor::getComponentName() const {
  return std::static_pointer_cast<std::string const>(this->flavor_)->c_str();
}

} // namespace facebook::react
