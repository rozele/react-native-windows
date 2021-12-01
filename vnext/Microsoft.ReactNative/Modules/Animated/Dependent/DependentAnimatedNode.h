// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace Microsoft::ReactNative {
class DependentNativeAnimatedNodeManager;
class DependentAnimatedNode {
 public:
  int64_t activeIncomingNodes;
  int64_t bfsColor;

  DependentAnimatedNode(int64_t tag, const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  int64_t Tag();
  void AddChild(int64_t animatedNode);
  void RemoveChild(int64_t animatedNode);

  virtual void Update() noexcept {};
  virtual void OnDetachedFromNode(int64_t /*animatedNodeTag*/){};
  virtual void OnAttachToNode(int64_t /*animatedNodeTag*/){};
  virtual ~DependentAnimatedNode() = default;

  std::vector<int64_t>& Children() {
    return m_children;
  }

 protected:
  DependentAnimatedNode *GetChildNode(int64_t tag);
  int64_t m_tag{0};
  const std::weak_ptr<DependentNativeAnimatedNodeManager> m_manager;
  std::vector<int64_t> m_children{};
};
} // namespace Microsoft::ReactNative
