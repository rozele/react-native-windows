// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "HyperlinkTextViewManager.h"
#include "KeyboardEventHandler.h"

#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <winrt/Windows.System.h>

namespace winrt {
using namespace xaml::Documents;
} // namespace winrt

namespace react::uwp {

struct KeyPressState {
  std::optional<winrt::Windows::System::VirtualKey> lastKey;
  xaml::UIElement::PreviewKeyUp_revoker keyUpRevoker;
};

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

  // Pointer click events should be handled by the TouchEventHandler. The only
  // condition where we want to send "onClick" events is when the user invokes
  // the hyperlink while it has focus by pressing "Enter" or "Space".
  const auto keyPressState = std::make_shared<KeyPressState>();
  hyperlink.GotFocus([keyPressState](auto&& sender, auto&&) {
    const auto hyperlink = sender.as<xaml::Documents::Hyperlink>();
    const auto textBlock = hyperlink.ContentStart().VisualParent().try_as<xaml::Controls::TextBlock>();
    if (textBlock) {
      keyPressState->keyUpRevoker = textBlock.PreviewKeyUp(
          winrt::auto_revoke,
          [keyPressState](auto&&, xaml::Input::KeyRoutedEventArgs const &args) {
            keyPressState->lastKey = args.Key();
          });
    }
  });

  hyperlink.LostFocus([keyPressState](auto &&...) {
    keyPressState->keyUpRevoker.revoke();
    keyPressState->lastKey = std::nullopt;
  });

  hyperlink.Click([=](auto&& ...) {
    const auto instance = GetReactInstance().lock();
    if (instance == nullptr)
      return;

    const auto lastKey = keyPressState->lastKey;
    if (lastKey == winrt::Windows::System::VirtualKey::Enter || lastKey == winrt::Windows::System::VirtualKey::Space) {
      keyPressState->lastKey = std::nullopt;
      folly::dynamic eventData = folly::dynamic::object("target", tag)("key", KeyboardHelper::CodeFromVirtualKey(lastKey.value()));
      instance->DispatchEvent(tag, "topClick", std::move(eventData));
    }
  });

  return hyperlink;
}

} // namespace react::uwp
