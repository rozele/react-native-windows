// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentDiffClampAnimatedNode final : public DependentValueAnimatedNode {
  using Super = DependentValueAnimatedNode;

 public:
  DependentDiffClampAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void Update() noexcept override;

 private:
  int64_t m_inputNodeTag{};
  double m_min{};
  double m_max{};
  double m_lastValue{};
};
} // namespace Microsoft::ReactNative
