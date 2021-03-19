// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ScrollContentViewManager.h"

#include "Impl/SnapPointManagingContentControl.h"
#include "ViewPanel.h"

namespace react::uwp {

ScrollContentViewManager::ScrollContentViewManager(const std::shared_ptr<IReactInstance> &reactInstance)
    : Super(reactInstance) {}

const char *ScrollContentViewManager::GetName() const {
  return "RCTScrollContentView";
}

XamlView ScrollContentViewManager::CreateViewCore(int64_t /*tag*/) {
  auto panel = winrt::make<winrt::react::uwp::implementation::ViewPanel>();
  panel.VerticalAlignment(xaml::VerticalAlignment::Stretch);
  panel.HorizontalAlignment(xaml::HorizontalAlignment::Stretch);
  return panel.as<XamlView>();
}

void ScrollContentViewManager::AddView(const XamlView &parent, const XamlView &child, int64_t index) {
  // All top-level children of inverted ScrollView content will be anchor candidates, unless scrolled to the top.
  auto childElement = child.as<xaml::UIElement>();
  auto scrollViewContentControl = parent.as<xaml::FrameworkElement>().Parent().as<SnapPointManagingContentControl>();
  if (scrollViewContentControl && scrollViewContentControl->IsInverted() && !scrollViewContentControl->IsScrolledToTop()) {
    childElement.CanBeScrollAnchor(true);
  }

  parent.as<winrt::react::uwp::ViewPanel>().InsertAt(static_cast<uint32_t>(index), childElement);
}

void ScrollContentViewManager::RemoveAllChildren(const XamlView &parent) {
  parent.as<winrt::react::uwp::ViewPanel>().Clear();
}

void ScrollContentViewManager::RemoveChildAt(const XamlView &parent, int64_t index) {
  parent.as<winrt::react::uwp::ViewPanel>().RemoveAt(static_cast<uint32_t>(index));
}

} // namespace react::uwp
