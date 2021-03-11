// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "ScrollContentViewManager.h"

namespace Microsoft::ReactNative {

ScrollContentViewManager::ScrollContentViewManager(const Mso::React::IReactContext &context) : Super(context) {}

const wchar_t *ScrollContentViewManager::GetName() const {
  return L"RCTScrollContentView";
}

void ViewViewManager::SetLayoutProps(
    ShadowNodeBase &nodeToUpdate,
    const XamlView &viewToUpdate,
    float left,
    float top,
    float width,
    float height) {
  // TODO: Could we do this only if a prop is set to maintain content position (or anchorRatio is set to 1.0)?
  Super::SetLayoutProps(nodeToUpdate, viewToUpdate, left, top, width, height);
  auto scrollViewContentControl = viewToUpdate.as<xaml::FrameworkElement>().Parent();
  auto scrollViewer = scrollViewContentControl.as<xaml::FrameworkElement>().Parent();
  scrollViewer.as<xaml::UIElement>().InvalidateArrange();
}

} // namespace Microsoft::ReactNative
