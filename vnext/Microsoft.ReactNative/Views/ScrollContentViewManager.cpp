// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ScrollContentViewManager.h"

#include "ViewPanel.h"

namespace react::uwp {

ScrollContentViewManager::ScrollContentViewManager(const std::shared_ptr<IReactInstance> &reactInstance)
    : Super(reactInstance) {}

const char *ScrollContentViewManager::GetName() const {
  return "RCTScrollContentView";
}

XamlView ScrollContentViewManager::CreateViewCore(int64_t /*tag*/) {
  return winrt::make<winrt::react::uwp::implementation::ViewPanel>();
}

void ScrollContentViewManager::AddView(const XamlView &parent, const XamlView &child, int64_t index) {
  // All top-level children of the ScrollViewer content panel will be anchor candidates.
  // TODO(T86782781): Pass a prop setting to the ScrollViewContent component in JS to enable / disable default scroll anchoring.
  auto childElement = child.as<xaml::UIElement>();
  childElement.CanBeScrollAnchor(true);
  parent.as<winrt::react::uwp::ViewPanel>().InsertAt(static_cast<uint32_t>(index), childElement);
}

void ScrollContentViewManager::RemoveAllChildren(const XamlView &parent) {
  parent.as<winrt::react::uwp::ViewPanel>().Clear();
}

void ScrollContentViewManager::RemoveChildAt(const XamlView &parent, int64_t index) {
  parent.as<winrt::react::uwp::ViewPanel>().RemoveAt(static_cast<uint32_t>(index));
}

} // namespace react::uwp
