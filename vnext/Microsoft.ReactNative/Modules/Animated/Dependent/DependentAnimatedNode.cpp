// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentAnimatedNode::DependentAnimatedNode(int64_t tag, const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : m_tag(tag), m_manager(manager) {}

int64_t DependentAnimatedNode::Tag() {
  return m_tag;
}

void DependentAnimatedNode::AddChild(const int64_t animatedNodeTag) {
  m_children.push_back(animatedNodeTag);
  GetChildNode(animatedNodeTag)->OnAttachToNode(m_tag);
}

void DependentAnimatedNode::RemoveChild(const int64_t tag) {
  if (const auto childNode = GetChildNode(tag)) {
    childNode->OnDetachedFromNode(m_tag);
    m_children.erase(std::find(m_children.begin(), m_children.end(), tag));
  }
}

DependentAnimatedNode *DependentAnimatedNode::GetChildNode(int64_t tag) {
  if (std::find(m_children.begin(), m_children.end(), tag) != m_children.end()) {
    if (const auto manager = m_manager.lock()) {
      return manager->GetAnimatedNode(tag);
    }
  }

  return static_cast<DependentAnimatedNode *>(nullptr);
}
} // namespace Microsoft::ReactNative
