// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentModulusAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentModulusAnimatedNode::DependentModulusAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  m_inputNodeTag = static_cast<int64_t>(config.find("input").dereference().second.asDouble());
  m_modulus = config.find("modulus").dereference().second.asDouble();
}

void DependentModulusAnimatedNode::Update() noexcept {
  if (const auto manager = m_manager.lock()) {
    if (const auto node = manager->GetValueAnimatedNode(m_inputNodeTag)) {
      RawValue(std::fmod(node->Value(), m_modulus));
    }
  }
}

} // namespace Microsoft::ReactNative
