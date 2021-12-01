// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentMultiplicationAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentMultiplicationAnimatedNode::DependentMultiplicationAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &inputNode : config.find("input").dereference().second) {
    m_inputNodes.insert(static_cast<int64_t>(inputNode.asDouble()));
  }
}

void DependentMultiplicationAnimatedNode::Update() noexcept {
  auto rawValue = 1.0;
  for (const auto &tag : m_inputNodes) {
    if (const auto manager = m_manager.lock()) {
      if (const auto node = manager->GetValueAnimatedNode(tag)) {
        rawValue *= node->Value();
      }
    }
  }
  RawValue(rawValue);
}

} // namespace Microsoft::ReactNative
