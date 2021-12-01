// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <IReactInstance.h>
#include <React.h>
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"
#include "JSValue.h"

namespace Microsoft::ReactNative {
class DependentPropsAnimatedNode final : public DependentAnimatedNode {
  using Super = DependentAnimatedNode;

 public:
  DependentPropsAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const Mso::CntPtr<Mso::React::IReactContext> &context,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  void ConnectToView(int64_t viewTag);
  void DisconnectFromView(int64_t viewTag);
  void RestoreDefaultValues();
  void UpdateView();
  void Commit();

 private:
  Mso::CntPtr<Mso::React::IReactContext> m_context{};
  std::unordered_map<std::string, int64_t> m_propNodeMapping{};
  winrt::Microsoft::ReactNative::JSValueObject m_props{};
  int64_t m_connectedViewTag{s_connectedViewTagUnset};

  static constexpr int64_t s_connectedViewTagUnset{-1};
};
} // namespace Microsoft::ReactNative
