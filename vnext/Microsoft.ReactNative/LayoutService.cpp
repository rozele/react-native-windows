// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "LayoutService.h"
#include "LayoutService.g.cpp"
#include <Modules/NativeUIManager.h>
#include <Modules/PaperUIManagerModule.h>

#ifdef USE_WINUI_FABRIC
#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#include <Fabric/WinUI/FabricUIManagerModule.h>
#endif

namespace winrt::Microsoft::ReactNative::implementation {

LayoutService::LayoutService(Mso::CntPtr<Mso::React::IReactContext> &&context) noexcept : m_context(context) {}

/*static*/ winrt::Microsoft::ReactNative::LayoutService LayoutService::FromContext(IReactContext context) {
  return context.Properties()
      .Get(LayoutService::LayoutServiceProperty().Handle())
      .try_as<winrt::Microsoft::ReactNative::LayoutService>();
}

/*static*/ ReactPropertyId<LayoutService> LayoutService::LayoutServiceProperty() noexcept {
  static ReactPropertyId<LayoutService> layoutServiceProperty{L"ReactNative.UIManager", L"LayoutService"};
  return layoutServiceProperty;
}

void LayoutService::ApplyLayoutForAllNodes() noexcept {
  if (auto uiManager = ::Microsoft::ReactNative::GetNativeUIManager(*m_context).lock()) {
    uiManager->DoLayout();
  }
}

void LayoutService::ApplyLayout(int64_t reactTag, float width, float height) noexcept {
#ifdef USE_WINUI_FABRIC
  if (const auto fabricUIManager =
          ::Microsoft::ReactNative::FabricUIManager::FromProperties(ReactPropertyBag(m_context->Properties()))) {
    if (const auto baseComponentView = std::static_pointer_cast<::Microsoft::ReactNative::BaseComponentView>(
            fabricUIManager->GetViewRegistry().findComponentViewWithTag(static_cast<facebook::react::Tag>(reactTag)))) {
      // TODO(T146190459): Wire up LayoutService functionality for Fabric
      return;
    }
  }
#endif
  if (auto uiManager = ::Microsoft::ReactNative::GetNativeUIManager(*m_context).lock()) {
    uiManager->ApplyLayout(reactTag, width, height);
  }
}

bool LayoutService::IsInBatch() noexcept {
  if (auto uiManager = ::Microsoft::ReactNative::GetNativeUIManager(*m_context).lock()) {
    return uiManager->isInBatch();
  }

  return false;
}

void LayoutService::MarkDirty(int64_t reactTag) noexcept {
  if (auto uiManager = ::Microsoft::ReactNative::GetNativeUIManager(*m_context).lock()) {
    uiManager->DirtyYogaNode(reactTag);
  }
}

} // namespace winrt::Microsoft::ReactNative::implementation
