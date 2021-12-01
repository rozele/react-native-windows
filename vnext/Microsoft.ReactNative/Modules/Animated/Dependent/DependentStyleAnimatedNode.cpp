// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentNativeAnimatedNodeManager.h"
#include "DependentStyleAnimatedNode.h"
#include "DependentTransformAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentStyleAnimatedNode::DependentStyleAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &entry : config.find("style").dereference().second.items()) {
    m_propMapping.insert({entry.first.getString(), static_cast<int64_t>(entry.second.asDouble())});
  }
}

void DependentStyleAnimatedNode::CollectViewUpdates(winrt::Microsoft::ReactNative::JSValueObject &props) noexcept {
  auto rawValue = 0.0;
  for (const auto &propMapping : m_propMapping) {
    if (const auto manager = m_manager.lock()) {
      if (const auto transformNode = manager->GetTransformAnimatedNode(propMapping.second)) {
        transformNode->CollectViewUpdates(props);
      } else if (const auto node = manager->GetValueAnimatedNode(propMapping.second)) {
        // TODO: auto
        const double value = node->Value();
        props[propMapping.first] = value;
      }
    }
  }
}
} // namespace Microsoft::ReactNative
