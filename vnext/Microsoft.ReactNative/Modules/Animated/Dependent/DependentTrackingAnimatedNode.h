// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentTrackingAnimatedNode final : public DependentAnimatedNode {
  using Super = DependentAnimatedNode;

 public:
  DependentTrackingAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void Update() noexcept override;

 private:
  int64_t m_animationId{};
  int64_t m_toValueId{};
  int64_t m_valueId{};
  folly::dynamic m_animationConfig{};

  static constexpr int64_t s_nodeTagUnset{-1};
};
} // namespace Microsoft::ReactNative
