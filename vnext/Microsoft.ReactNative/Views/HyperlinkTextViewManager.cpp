// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "HyperlinkTextViewManager.h"
#include "KeyboardEventHandler.h"

#include <JSValueWriter.h>
#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <winrt/Windows.System.h>

namespace winrt {
using namespace xaml::Documents;
} // namespace winrt

namespace Microsoft::ReactNative {

struct KeyPressState {
  std::optional<winrt::Windows::System::VirtualKey> lastKey;
  xaml::UIElement::PreviewKeyUp_revoker keyUpRevoker;
};


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
  hyperlink.UnderlineStyle(winrt::UnderlineStyle::None);

  // Pointer click events should be handled by the TouchEventHandler. The only
  // condition where we want to send "onClick" events is when the user invokes
  // the hyperlink while it has focus by pressing "Enter" or "Space".
  const auto keyPressState = std::make_shared<KeyPressState>();
  hyperlink.GotFocus([=](auto&& sender, auto&&) {
    const auto hyperlink = sender.as<xaml::Documents::Hyperlink>();
    const auto textBlock = hyperlink.ContentStart().VisualParent().as<xaml::Controls::TextBlock>();
    keyPressState->keyUpRevoker = textBlock.PreviewKeyUp(
        winrt::auto_revoke,
        [=](auto&&, xaml::Input::KeyRoutedEventArgs const &args) {
          keyPressState->lastKey = args.Key();
        });
    });

  hyperlink.LostFocus([=](auto &&...) {
    keyPressState->keyUpRevoker.revoke();
    keyPressState->lastKey = std::nullopt;
  });

  hyperlink.Click([=](auto &&sender, auto&&) {
    const auto lastKey = keyPressState->lastKey;
    if (lastKey == winrt::Windows::System::VirtualKey::Enter || lastKey == winrt::Windows::System::VirtualKey::Space) {
      keyPressState->lastKey = std::nullopt;
      folly::dynamic eventData = folly::dynamic::object("target", tag)("key", KeyboardHelper::CodeFromVirtualKey(lastKey.value()));
      GetReactContext().DispatchEvent(tag, "topClick", std::move(eventData));
    }
  });

  return hyperlink;
}

} // namespace Microsoft::ReactNative
