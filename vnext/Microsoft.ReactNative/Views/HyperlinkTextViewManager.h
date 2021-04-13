// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Views/VirtualTextViewManager.h>

namespace react::uwp {

class HyperlinkTextViewManager : public VirtualTextViewManager {
  using Super = VirtualTextViewManager;

 public:
  HyperlinkTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;
  facebook::react::ShadowNode *createShadow() const override {
    auto shadowNode = new VirtualTextShadowNode();
    // TODO(T88837050): Remove once Windows RN JS includes logic to send `isPressable` prop
    shadowNode->m_isPressable = true;
    shadowNode->m_pressableCount = 1;
    return shadowNode;
  }

  folly::dynamic GetExportedCustomDirectEventTypeConstants() const override;

 protected:
  XamlView CreateViewCore(int64_t tag) override;
};

} // namespace react::uwp
