// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentNativeAnimatedNodeManager.h"
#include "DependentTrackingAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentTrackingAnimatedNode::DependentTrackingAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  m_animationId = static_cast<int64_t>(config.find("animationId").dereference().second.asDouble());
  m_toValueId = static_cast<int64_t>(config.find("toValue").dereference().second.asDouble());
  m_valueId = static_cast<int64_t>(config.find("value").dereference().second.asDouble());
  m_animationConfig = std::move(config.find("animationConfig").dereference().second);
}

void DependentTrackingAnimatedNode::Update() noexcept {
  if (const auto manager = m_manager.lock()) {
    if (const auto node = manager->GetValueAnimatedNode(m_toValueId)) {
      m_animationConfig["toValue"] = node->Value();
      manager->StartAnimatingNode(m_animationId, m_valueId, m_animationConfig, nullptr, manager);
    }
  }
}

} // namespace Microsoft::ReactNative
