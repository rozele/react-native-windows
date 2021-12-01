// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentNativeAnimatedNodeManager.h"
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {

DependentValueAnimatedNode::DependentValueAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  m_value = config.find("value").dereference().second.asDouble();
  m_offset = config.find("offset").dereference().second.asDouble();
}

DependentValueAnimatedNode::DependentValueAnimatedNode(
    int64_t tag,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {}

} // namespace Microsoft::ReactNative
