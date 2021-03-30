// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "HyperlinkTextViewManager.h"

#include <JSValueWriter.h>
#include <UI.Xaml.Documents.h>

namespace winrt {
using namespace xaml::Documents;
} // namespace winrt

namespace Microsoft::ReactNative {

HyperlinkTextViewManager::HyperlinkTextViewManager(const Mso::React::IReactContext &context) : Super(context) {}

const wchar_t *HyperlinkTextViewManager::GetName() const {
  return L"RCTHyperlinkText";
}

void HyperlinkTextViewManager::GetExportedCustomDirectEventTypeConstants(
    const winrt::Microsoft::ReactNative::IJSValueWriter &writer) const {
  Super::GetExportedCustomDirectEventTypeConstants(writer);

  writer.WritePropertyName(L"topClick");
  writer.WriteObjectBegin();
  winrt::Microsoft::ReactNative::WriteProperty(writer, L"registrationName", L"onClick");
  writer.WriteObjectEnd();
}

XamlView HyperlinkTextViewManager::CreateViewCore(
    int64_t tag,
    const winrt::Microsoft::ReactNative::JSValueObject &) {
  winrt::Hyperlink hyperlink;

  // Underline should be handled by base class using TextDecorations
  // TODO: should we also override platform defaults for foreground?
  hyperlink.UnderlineStyle(winrt::UnderlineStyle::None);

  hyperlink.Click([=](auto &&, auto &&) {
    folly::dynamic eventData = folly::dynamic::object("target", tag);
    GetReactContext().DispatchEvent(tag, "topClick", std::move(eventData));
  });

  return hyperlink;
}

} // namespace Microsoft::ReactNative
