// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"
#include "JSValue.h"

namespace Microsoft::ReactNative {
struct DependentTransformConfig {
 public:
  std::string property;
  int64_t nodeTag;
  double value;
};
class DependentTransformAnimatedNode final : public DependentAnimatedNode {
  using Super = DependentAnimatedNode;

 public:
  DependentTransformAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void CollectViewUpdates(winrt::Microsoft::ReactNative::JSValueObject &props) noexcept;

 private:
  std::vector<DependentTransformConfig> m_transformConfigs;

  static constexpr int64_t s_nodeTagUnset{-1};
};
} // namespace Microsoft::ReactNative
