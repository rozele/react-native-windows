// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <Modules/NativeUIManager.h>
#include <Modules/PaperUIManagerModule.h>
#include "DependentPropsAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentPropsAnimatedNode::DependentPropsAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const Mso::CntPtr<Mso::React::IReactContext> &context,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  m_context = context;
  for (const auto &entry : config.find("props").dereference().second.items()) {
    m_propNodeMapping.insert({entry.first.getString(), static_cast<int64_t>(entry.second.asDouble())});
  }
}


void DependentPropsAnimatedNode::ConnectToView(int64_t viewTag) {
  // TODO: asserts
  m_connectedViewTag = viewTag;
}

void DependentPropsAnimatedNode::DisconnectFromView(int64_t viewTag) {
  // TODO: asserts
  m_connectedViewTag = s_connectedViewTagUnset;
}

void DependentPropsAnimatedNode::RestoreDefaultValues() {
  for (const auto &entry : m_props) {
    m_props[entry.first] = nullptr;
  }

  Commit();
}

void DependentPropsAnimatedNode::UpdateView() {
  if (m_connectedViewTag == s_connectedViewTagUnset) {
    return;
  }

  for (const auto &entry : m_propNodeMapping) {
    if (const auto manager = m_manager.lock()) {
      if (const auto styleNode = manager->GetStyleAnimatedNode(entry.second)) {
        styleNode->CollectViewUpdates(m_props);
      } else if (const auto node = manager->GetValueAnimatedNode(entry.second)) {
        m_props[entry.first] = node->Value();
      }
    }
  }

  Commit();
}

void DependentPropsAnimatedNode::Commit() {
  if (const auto uiManager = GetNativeUIManager(*m_context).lock()) {
    uiManager->ensureInBatch();
    if (const auto node = uiManager->getHost()->FindShadowNodeForTag(m_connectedViewTag)) {
      if (!node->m_zombie)
        node->updateProperties(m_props);

      uiManager->UpdateView(*node, m_props);
    }
  }
}

} // namespace Microsoft::ReactNative
