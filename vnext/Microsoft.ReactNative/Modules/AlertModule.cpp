// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "AlertModule.h"
#include "Unicode.h"

#include <Utils/ValueUtils.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include "Utils/Helpers.h"

#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Media.h>

namespace Microsoft::ReactNative {

void Alert::showAlert(ShowAlertArgs const &args, std::function<void(std::string)> result) noexcept {
  auto jsDispatcher = m_context.JSDispatcher();
  m_context.UIDispatcher().Post([weakThis = weak_from_this(), jsDispatcher, result, args] {
    if (auto strongThis = weakThis.lock()) {
      xaml::Controls::ContentDialog dialog{};
      dialog.Title(winrt::box_value(Microsoft::Common::Unicode::Utf8ToUtf16(args.title)));
      dialog.Content(winrt::box_value(Microsoft::Common::Unicode::Utf8ToUtf16(args.message)));
      dialog.PrimaryButtonText(Microsoft::Common::Unicode::Utf8ToUtf16(args.buttonPositive));
      dialog.SecondaryButtonText(Microsoft::Common::Unicode::Utf8ToUtf16(args.buttonNegative));
      dialog.CloseButtonText(Microsoft::Common::Unicode::Utf8ToUtf16(args.buttonNeutral));
      if (!args.buttonPositive.empty()) {
        dialog.DefaultButton(winrt::Windows::UI::Xaml::Controls::ContentDialogButton::Primary);
      } else if (!args.buttonNegative.empty()) {
        dialog.DefaultButton(winrt::Windows::UI::Xaml::Controls::ContentDialogButton::Secondary);
      } else {
        dialog.DefaultButton(winrt::Windows::UI::Xaml::Controls::ContentDialogButton::Close);
      }

      if (react::uwp::Is19H1OrHigher()) {
        // XamlRoot added in 19H1
        if (auto xamlRoot = React::XamlUIService::GetXamlRoot(strongThis->m_context.Properties().Handle())) {
          dialog.XamlRoot(xamlRoot);
        }

        // Workaround XAML bug with ContentDialog and dark theme:
        // https://github.com/microsoft/microsoft-ui-xaml/issues/2331
        dialog.Opened([](winrt::IInspectable const &sender, auto &&) {
          auto contentDialog = sender.as<xaml::Controls::ContentDialog>();
          auto popups = xaml::Media::VisualTreeHelper::GetOpenPopupsForXamlRoot(contentDialog.XamlRoot());
          auto xamlRootContentAsFrameworkElement = contentDialog.XamlRoot().Content().try_as<xaml::FrameworkElement>();
          if (xamlRootContentAsFrameworkElement) {
            for (auto i = 0; i < popups.Size(); i++) {
              popups.GetAt(i).RequestedTheme(xamlRootContentAsFrameworkElement.ActualTheme());
            }
          }
        });
      }

      auto asyncOp = dialog.ShowAsync();
      asyncOp.Completed(
          [jsDispatcher, result](
              const winrt::IAsyncOperation<xaml::Controls::ContentDialogResult> &asyncOp, winrt::AsyncStatus status) {
            switch (asyncOp.GetResults()) {
              case xaml::Controls::ContentDialogResult::Primary:
                jsDispatcher.Post([result] { result("positive"); });
                break;
              case xaml::Controls::ContentDialogResult::Secondary:
                jsDispatcher.Post([result] { result("negative"); });
                break;
              case xaml::Controls::ContentDialogResult::None:
                jsDispatcher.Post([result] { result("neutral"); });
                break;
              default:
                break;
            }
          });
    }
  });
}

void Alert::Initialize(React::ReactContext const &reactContext) noexcept {
  m_context = reactContext;
}

} // namespace Microsoft::ReactNative
