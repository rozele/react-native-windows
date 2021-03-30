// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "HyperlinkTextViewManager.h"

#include <UI.Xaml.Documents.h>

namespace winrt {
using namespace xaml::Documents;
} // namespace winrt

namespace react::uwp {

HyperlinkTextViewManager::HyperlinkTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance)
    : Super(reactInstance) {}

const char *HyperlinkTextViewManager::GetName() const {
  return "RCTHyperlinkText";
}

folly::dynamic HyperlinkTextViewManager::GetExportedCustomDirectEventTypeConstants() const {
  auto directEvents = Super::GetExportedCustomDirectEventTypeConstants();
  directEvents["topClick"] = folly::dynamic::object("registrationName", "onClick");
  return directEvents;
}

XamlView HyperlinkTextViewManager::CreateViewCore(int64_t tag) {
  winrt::Hyperlink hyperlink;

  // Underline should be handled by base class using TextDecorations
  hyperlink.UnderlineStyle(winrt::UnderlineStyle::None);

  hyperlink.Click([=](auto &&, auto &&) {
    const auto instance = GetReactInstance().lock();
    if (instance == nullptr)
      return;

    folly::dynamic eventData = folly::dynamic::object("target", tag);
    instance->DispatchEvent(tag, "topClick", std::move(eventData));
  });

  return hyperlink;
}

} // namespace react::uwp
