// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"
#include "JSValue.h"

namespace Microsoft::ReactNative {
class DependentStyleAnimatedNode final : public DependentAnimatedNode {
  using Super = DependentAnimatedNode;

 public:
  DependentStyleAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);

  void CollectViewUpdates(winrt::Microsoft::ReactNative::JSValueObject &props) noexcept;

 private:
  std::unordered_map<std::string, int64_t> m_propMapping{};
};
} // namespace Microsoft::ReactNative
