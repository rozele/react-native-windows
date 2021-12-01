// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentModulusAnimatedNode final : public DependentValueAnimatedNode {
  using Super = DependentValueAnimatedNode;

 public:
  DependentModulusAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void Update() noexcept override;

 private:
  int64_t m_inputNodeTag{};
  double m_modulus{};
};
} // namespace Microsoft::ReactNative
