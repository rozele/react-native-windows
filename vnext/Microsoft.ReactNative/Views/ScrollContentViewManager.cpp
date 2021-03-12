// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ScrollContentViewManager.h"

#include "ShadowNodeBase.h"
#include "ViewPanel.h"

#include <unicode.h>

namespace Microsoft::ReactNative {

ScrollContentViewManager::ScrollContentViewManager(const Mso::React::IReactContext &context) : Super(context) {}

const wchar_t *ScrollContentViewManager::GetName() const {
  return L"RCTScrollContentView";
}

XamlView ScrollContentViewManager::CreateViewCore(
    int64_t /*tag*/,
    const winrt::Microsoft::ReactNative::JSValueObject &) {
  return winrt::make<winrt::react::uwp::implementation::ViewPanel>();
}

void ScrollContentViewManager::AddView(const XamlView &parent, const XamlView &child, [[maybe_unused]] int64_t index) {
  // All top-level children of the ScrollViewer content panel will be anchor candidates.
  // TODO: Pass a prop setting to the ScrollViewContent component in JS to enable / disable scroll anchoring.
  auto childElement = child.as<xaml::UIElement>();
  childElement.CanBeScrollAnchor(true);

  auto panel = parent.as<winrt::react::uwp::ViewPanel>();
  panel.InsertAt(static_cast<uint32_t>(index), childElement);

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

void ScrollContentViewManager::RemoveAllChildren(const XamlView &parent) {
  parent.as<winrt::react::uwp::ViewPanel>().Clear();
}

void ScrollContentViewManager::RemoveChildAt(const XamlView &parent, [[maybe_unused]] int64_t index) {
  parent.as<winrt::react::uwp::ViewPanel>().RemoveAt(static_cast<uint32_t>(index));
}


} // namespace Microsoft::ReactNative
