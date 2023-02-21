// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>

#include <folly/dynamic.h>
#include <react/renderer/components/view/ViewEventEmitter.h>
#include <react/renderer/core/EventEmitter.h>

namespace facebook::react {

class LegacyABIViewEventEmitter;

using SharedLegacyABIViewEventEmitter = std::shared_ptr<const LegacyABIViewEventEmitter>;

class LegacyABIViewEventEmitter : public ViewEventEmitter {
 public:
  using ViewEventEmitter::ViewEventEmitter;

  void dispatchEvent(std::string const &type, folly::dynamic const &payload) const;
};

} // namespace facebook::react
