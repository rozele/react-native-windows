// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentNativeAnimatedNodeManager.h"
#include "DependentAdditionAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentAdditionAnimatedNode::DependentAdditionAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &inputNode : config.find("input").dereference().second) {
    m_inputNodes.insert(static_cast<int64_t>(inputNode.asDouble()));
  }
}

void DependentAdditionAnimatedNode::Update() noexcept {
  auto rawValue = 0.0;
  for (const auto &tag : m_inputNodes) {
    if (const auto manager = m_manager.lock()) {
      if (const auto node = manager->GetValueAnimatedNode(tag)) {
        rawValue += node->Value();
      }
    }
  }
  RawValue(rawValue);
}

} // namespace Microsoft::ReactNative
