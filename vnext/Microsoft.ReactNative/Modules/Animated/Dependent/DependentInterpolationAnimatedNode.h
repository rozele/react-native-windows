// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentInterpolationAnimatedNode final : public DependentValueAnimatedNode {
  using Super = DependentValueAnimatedNode;

 public:
  DependentInterpolationAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void OnDetachedFromNode(int64_t animatedNodeTag) override;
  void OnAttachToNode(int64_t animatedNodeTag) override;

  void Update() noexcept override;

 private:
  std::vector<double> m_inputRanges;
  std::vector<double> m_outputRanges;
  std::string m_extrapolateLeft;
  std::string m_extrapolateRight;

  int64_t m_parentTag{s_parentTagUnset};

  static constexpr int64_t s_parentTagUnset{-1};
};
} // namespace Microsoft::ReactNative
