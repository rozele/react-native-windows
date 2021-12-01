// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentDivisionAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentDivisionAnimatedNode::DependentDivisionAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &inputNode : config.find("input").dereference().second) {
    m_inputNodes.insert(static_cast<int64_t>(inputNode.asDouble()));
  }
}

void DependentDivisionAnimatedNode::Update() noexcept {
  auto initialValueSet = false;
  auto rawValue = 0.0;
  for (const auto &tag : m_inputNodes) {
    if (const auto manager = m_manager.lock()) {
      if (const auto node = manager->GetValueAnimatedNode(tag)) {
        const auto value = node->Value();
        rawValue = initialValueSet ? rawValue / value : value;
        initialValueSet = true;
      }
    }
  }
  RawValue(rawValue);
}

} // namespace Microsoft::ReactNative
