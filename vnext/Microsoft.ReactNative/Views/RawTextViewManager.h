// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Views/ShadowNodeBase.h>
#include <Views/ViewManagerBase.h>
#include <folly/dynamic.h>

namespace react::uwp {

struct RawTextShadowNode final : public ShadowNodeBase {
  using Super = ShadowNodeBase;
  winrt::hstring originalText{};
};

class RawTextViewManager : public ViewManagerBase {
  using Super = ViewManagerBase;

 public:
  RawTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;
  facebook::react::ShadowNode *createShadow() const override {
    return new RawTextShadowNode();
  }

  void SetLayoutProps(
      ShadowNodeBase &nodeToUpdate,
      const XamlView &viewToUpdate,
      float left,
      float top,
      float width,
      float height) override;
  bool RequiresYogaNode() const override;

 protected:
  bool UpdateProperty(
      ShadowNodeBase *nodeToUpdate,
      const std::string &propertyName,
      const folly::dynamic &propertyValue) override;

  XamlView CreateViewCore(int64_t tag) override;

 private:
  void NotifyAncestorsTextChanged(ShadowNodeBase *nodeToUpdate);
};

} // namespace react::uwp
