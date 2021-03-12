// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ScrollContentViewManager.h"

#include "ShadowNodeBase.h"
#include "ViewPanel.h"

#include <unicode.h>

namespace Microsoft::ReactNative {

// ScrollContentViewShadowNode

class ScrollContentViewShadowNode : public ShadowNodeBase {
  using Super = ShadowNodeBase;

 public:
  ScrollContentViewShadowNode() = default;

  void AddView(ShadowNode &child, int64_t index) override {
    auto panel = GetViewPanel();
    const auto &view = static_cast<ShadowNodeBase &>(child).GetView();
    const auto element = view.try_as<xaml::UIElement>();
    if (element == nullptr) {
      const auto &ii = view.as<winrt::IInspectable>();
      auto name = winrt::get_class_name(ii);
      YellowBox(
          std::string("ViewViewManager::AddView expected a UIElement but got a ") +
          Microsoft::Common::Unicode::Utf16ToUtf8(name.c_str()));
    }

    // All top-level children of the ScrollViewer content panel will be anchor candidates.
    // TODO: Pass a prop setting to the ScrollViewContent component in JS to enable / disable scroll anchoring.
    element.CanBeScrollAnchor(true);

    panel.InsertAt(static_cast<uint32_t>(index), element);

    auto scrollViewContentControl = panel.Parent();
    if (scrollViewContentControl) {
      auto scrollViewer = scrollViewContentControl.as<xaml::FrameworkElement>().Parent();
      if (scrollViewer) {
        // ScrollViewer selects an anchor during the Arrange phase of layout.
        // If you do not call InvalidateArrange whenever a new child is added
        // to the ScrollViewer content, the anchor behavior does not work.
        // TODO: Pass a prop setting to the ScrollViewContent component in JS to enable / disable scroll anchoring.
        scrollViewer.as<xaml::UIElement>().InvalidateArrange();
      }
    }
  }

  void RemoveChildAt(int64_t indexToRemove) override {
    if (indexToRemove == static_cast<uint32_t>(indexToRemove))
      GetViewPanel().RemoveAt(static_cast<uint32_t>(indexToRemove));
  }

  void removeAllChildren() override {
    GetViewPanel().Clear();
  }

  void ReplaceChild(const XamlView &oldChildView, const XamlView &newChildView) override {
    auto pPanel = GetViewPanel();
    if (pPanel != nullptr) {
      uint32_t index;
      if (pPanel.Children().IndexOf(oldChildView.as<xaml::UIElement>(), index)) {
        pPanel.RemoveAt(index);
        pPanel.InsertAt(index, newChildView.as<xaml::UIElement>());
      } else {
        assert(false);
      }
    }
  }

  winrt::react::uwp::ViewPanel GetViewPanel() {
    XamlView current = m_view;

    auto panel = current.try_as<winrt::react::uwp::ViewPanel>();
    assert(panel != nullptr);

    return panel;
  }
};

ScrollContentViewManager::ScrollContentViewManager(const Mso::React::IReactContext &context) : Super(context) {}

const wchar_t *ScrollContentViewManager::GetName() const {
  return L"RCTScrollContentView";
}

ShadowNode *ScrollContentViewManager::createShadow() const {
  return new ScrollContentViewShadowNode();
}

XamlView ScrollContentViewManager::CreateViewCore(
    int64_t /*tag*/,
    const winrt::Microsoft::ReactNative::JSValueObject &) {
  auto panel = winrt::make<winrt::react::uwp::implementation::ViewPanel>();
  panel.VerticalAlignment(xaml::VerticalAlignment::Stretch);
  panel.HorizontalAlignment(xaml::HorizontalAlignment::Stretch);

  return panel.as<XamlView>();
}

} // namespace Microsoft::ReactNative
