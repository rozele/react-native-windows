// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <INativeUIManager.h>
#include <UI.Xaml.Documents.h>
#include <Utils/TransformableText.h>
#include <Views/FrameworkElementViewManager.h>
#include <Views/ShadowNodeBase.h>

namespace react::uwp {

struct VirtualTextShadowNode final : public ShadowNodeBase {
  using Super = ShadowNodeBase;
  TransformableText transformableText{};

  void AddView(ShadowNode &child, int64_t index) override;

  struct HighlightData {
    std::vector<HighlightData> data;
    size_t spanIdx = 0;
    std::optional<winrt::Windows::UI::Color> color;
  };

  HighlightData m_highlightData;
};

class VirtualTextViewManager : public ViewManagerBase {
  using Super = ViewManagerBase;

 public:
  VirtualTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;
  facebook::react::ShadowNode *createShadow() const override {
    return new VirtualTextShadowNode();
  }

  void AddView(const XamlView &parent, const XamlView &child, int64_t index) override;
  void RemoveAllChildren(const XamlView &parent) override;
  void RemoveChildAt(const XamlView &parent, int64_t index) override;

  bool RequiresYogaNode() const override;

 protected:
  bool UpdateProperty(
      ShadowNodeBase *nodeToUpdate,
      const std::string &propertyName,
      const folly::dynamic &propertyValue) override;

  XamlView CreateViewCore(int64_t tag) override;
};

} // namespace react::uwp