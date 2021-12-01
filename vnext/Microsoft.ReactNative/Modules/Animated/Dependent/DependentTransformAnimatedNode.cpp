// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentNativeAnimatedNodeManager.h"
#include "DependentTransformAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentTransformAnimatedNode::DependentTransformAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &transform : config.find("transforms").dereference().second) {
    const auto property = transform.find("property").dereference().second.asString();
    if (transform.find("type").dereference().second.asString() == "animated") {
      m_transformConfigs.push_back(DependentTransformConfig{
          property, static_cast<int64_t>(transform.find("nodeTag").dereference().second.asDouble()), 0});
    } else {
      m_transformConfigs.push_back(
          DependentTransformConfig{property, s_nodeTagUnset, transform.find("value").dereference().second.asDouble()});
    }
  }
}

void DependentTransformAnimatedNode::CollectViewUpdates(winrt::Microsoft::ReactNative::JSValueObject &props) noexcept {
  winrt::Microsoft::ReactNative::JSValueArray transforms;
  for (const auto& transformConfig : m_transformConfigs) {
    std::optional<double> value;
    if (transformConfig.nodeTag == s_nodeTagUnset) {
      value = transformConfig.value;
    } else {
      if (const auto manager = m_manager.lock()) {
        if (const auto node = manager->GetValueAnimatedNode(transformConfig.nodeTag)) {
          value = node->Value();
        }
      }
    }

    if (value) {
      winrt::Microsoft::ReactNative::JSValueObject transform;
      transform[transformConfig.property] = value.value();
      transforms.emplace_back(std::move(transform));
    }
  }

  props["transform"] = std::move(transforms);
}

} // namespace Microsoft::ReactNative
