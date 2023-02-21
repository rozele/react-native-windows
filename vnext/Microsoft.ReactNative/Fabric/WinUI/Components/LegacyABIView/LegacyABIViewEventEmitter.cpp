// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "LegacyABIViewEventEmitter.h"

namespace facebook::react {

void LegacyABIViewEventEmitter::dispatchEvent(std::string const &type, folly::dynamic const &payload) const {
  EventEmitter::dispatchEvent(type, payload);
}

} // namespace facebook::react
