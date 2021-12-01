// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentDivisionAnimatedNode final : public DependentValueAnimatedNode {
  using Super = DependentValueAnimatedNode;

 public:
  DependentDivisionAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void Update() noexcept override;

 private:
  std::unordered_set<int64_t> m_inputNodes{};
};
} // namespace Microsoft::ReactNative
