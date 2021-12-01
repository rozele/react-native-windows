// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentDiffClampAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentDiffClampAnimatedNode::DependentDiffClampAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, config, manager) {
  m_inputNodeTag = static_cast<int64_t>(config.find("input").dereference().second.asDouble());
  m_min = config.find("min").dereference().second.asDouble();
  m_max = config.find("max").dereference().second.asDouble();
  m_lastValue = 0.0;
  RawValue(0.0);
}

void DependentDiffClampAnimatedNode::Update() noexcept {
  if (const auto manager = m_manager.lock()) {
    if (const auto node = manager->GetValueAnimatedNode(m_inputNodeTag)) {
      const auto value = node->Value();
      const auto diff = value - m_lastValue;
      m_lastValue = value;
      RawValue(std::clamp(Value() + diff, m_min, m_max));
    }
  }
}

} // namespace Microsoft::ReactNative
