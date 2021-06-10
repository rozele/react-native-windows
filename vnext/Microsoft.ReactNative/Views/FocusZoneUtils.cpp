// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"

#include "FocusZoneUtils.h"

#include "DynamicAutomationProperties.h"
#include "ViewControl.h"
#include "XamlView.h"

#include <Modules/NativeUIManager.h>
#include <Utils/AccessibilityUtils.h>
#include <Utils/PropertyHandlerUtils.h>
#include <Utils/PropertyUtils.h>

#include <INativeUIManager.h>
#include <IReactInstance.h>

#include <UI.Xaml.Controls.Primitives.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Interop.h>

namespace winrt {
using ContentControl = winrt::Windows::UI::Xaml::Controls::ContentControl;
using namespace Windows::Foundation;
using namespace xaml;
using namespace xaml::Controls;
using namespace xaml::Controls::Primitives;
using namespace xaml::Interop;
using namespace xaml::Input;
using namespace xaml::Media;
} // namespace winrt

static const std::unordered_map<std::string, xaml::Input::XYFocusNavigationStrategy> xyFocusNavigationStrategy = {
    {"projection", xaml::Input::XYFocusNavigationStrategy::Projection},
    {"navigationDirectionDistance", xaml::Input::XYFocusNavigationStrategy::NavigationDirectionDistance},
    {"rectilinearDistance", xaml::Input::XYFocusNavigationStrategy::RectilinearDistance}};

template <>
struct json_type_traits<xaml::Input::XYFocusNavigationStrategy> {
  static xaml::Input::XYFocusNavigationStrategy parseJson(const folly::dynamic &json) {
    auto iter = xyFocusNavigationStrategy.find(json.asString());

    if (iter != xyFocusNavigationStrategy.end()) {
      return iter->second;
    }

    return xaml::Input::XYFocusNavigationStrategy::Auto;
  }
};
namespace react::uwp {

bool IsElementChildOf(winrt::DependencyObject element, winrt::DependencyObject expectedParent) {
  winrt::DependencyObject parent = element;
  do {
    parent = winrt::VisualTreeHelper::GetParent(parent);
    if (parent == expectedParent) {
      return true;
    }
  } while (parent != nullptr);
  return false;
}

bool IsElementWithinAnotherFocusableElementInContainer(
    winrt::DependencyObject element,
    winrt::FrameworkElement container) {
  auto parent = element;
  do {
    parent = winrt::VisualTreeHelper::GetParent(parent);
    auto control = parent.try_as<winrt::Control>();
    if (control != nullptr && control.IsTabStop() && control.IsEnabled()) {
      return true;
    }
  } while (parent != nullptr && parent != container);
  return false;
}

const winrt::TypeName focusZoneTypeName{winrt::hstring{L"FocusZoneProperties"}, winrt::TypeKind::Metadata};

xaml::DependencyProperty FocusZoneLastFocusedElementTagProperty() {
  static xaml::DependencyProperty s_FocusZoneLastFocusedElementTagProperty = xaml::DependencyProperty::RegisterAttached(
      L"FocusZoneLastFocusedElementTag",
      winrt::xaml_typename<winrt::Windows::Foundation::IInspectable>(),
      focusZoneTypeName,
      winrt::PropertyMetadata(nullptr));

  return s_FocusZoneLastFocusedElementTagProperty;
}

xaml::DependencyProperty IsMovingFocusOutOfFocusZoneProperty() {
  static xaml::DependencyProperty s_IsMovingFocusOutOfFocusZoneProperty = xaml::DependencyProperty::RegisterAttached(
      L"IsMovingFocusOutOfFocusZone",
      winrt::xaml_typename<bool>(),
      focusZoneTypeName,
      winrt::PropertyMetadata(winrt::box_value(false)));

  return s_IsMovingFocusOutOfFocusZoneProperty;
}

winrt::ScrollViewer GetFirstChildScrollViewer(winrt::DependencyObject element) {
  if (auto scrollViewer = element.try_as<winrt::ScrollViewer>()) {
    return scrollViewer;
  }

  auto childrenCount = winrt::VisualTreeHelper::GetChildrenCount(element);
  for (auto i = 0; i < childrenCount; i++) {
    auto child = winrt::VisualTreeHelper::GetChild(element, i);
    if (auto scrollViewer = GetFirstChildScrollViewer(child)) {
      return scrollViewer;
    }
  }

  return nullptr;
}

void OnFocusZoneKeyDown(const winrt::IInspectable &sender, const winrt::KeyRoutedEventArgs &e) {
  // Xaml has a bug (https://github.com/microsoft/microsoft-ui-xaml/issues/1363)
  // with KeyboardNavigationMode::Once where shift+tab won't work properly if
  // focusable elements are nested inside other elements. This fixes that and
  // also tracks the last focused element.

  if (e.Key() == winrt::Windows::System::VirtualKey::Tab) {
    auto senderFrameworkElement = sender.as<xaml::FrameworkElement>();
    auto xamlRoot = senderFrameworkElement.XamlRoot();

    // With XYFocusKeyboardNavigation enabled, the way to focus a child of a
    // focusable element is with tab. Then shift+tab focuses the parent. If
    // we're in that case then let Xaml handle the shift+tab.
    auto focusedElement = winrt::FocusManager::GetFocusedElement(xamlRoot).try_as<winrt::DependencyObject>();
    if (IsElementWithinAnotherFocusableElementInContainer(focusedElement, senderFrameworkElement)) {
      return;
    }

    auto const &coreWindow = winrt::CoreWindow::GetForCurrentThread();
    auto isShiftDown = KeyboardHelper::IsModifiedKeyPressed(coreWindow, winrt::Windows::System::VirtualKey::Shift);

    if (isShiftDown) {
      if (auto scrollViewer = GetFirstChildScrollViewer(senderFrameworkElement)) {
        // Since we leave the FocusZoneView by moving focus backwards
        // repeatedly, we don't want any ScrollViewer to scroll during
        // this process.
        scrollViewer.BringIntoViewOnFocusChange(false);
      }
      winrt::FindNextElementOptions findNextElementOptions = winrt::FindNextElementOptions();
      findNextElementOptions.SearchRoot(xamlRoot.Content());

      auto didMoveFocus = false;
      do {
        didMoveFocus =
            winrt::FocusManager::TryMoveFocus(winrt::FocusNavigationDirection::Previous, findNextElementOptions);

        // As we move focus backwards through the focus zone, it triggers the
        // LosingFocus event which stores the last focused element. We don't want
        // to do this after the first time we move focus, so we set this flag.
        senderFrameworkElement.SetValue(IsMovingFocusOutOfFocusZoneProperty(), winrt::box_value(true));

        focusedElement = winrt::FocusManager::GetFocusedElement(xamlRoot).try_as<winrt::DependencyObject>();
      } while (didMoveFocus && IsElementChildOf(focusedElement, senderFrameworkElement));
      senderFrameworkElement.SetValue(IsMovingFocusOutOfFocusZoneProperty(), winrt::box_value(false));
      e.Handled(true);
    }
  }
}

void OnFocusZoneGettingFocus(
    const winrt::IInspectable &sender,
    const winrt::GettingFocusEventArgs &e,
    ViewManagerBase *viewManager) {
  auto senderDependencyObject = sender.as<winrt::DependencyObject>();
  auto isOldFocusedElementOutsideFocusZoneView =
      e.OldFocusedElement() != nullptr && !IsElementChildOf(e.OldFocusedElement(), senderDependencyObject);

  if (isOldFocusedElementOutsideFocusZoneView) {
    if (auto scrollViewer = GetFirstChildScrollViewer(senderDependencyObject)) {
      // Revert this property from above.
      scrollViewer.BringIntoViewOnFocusChange(true);
    }

    auto lastFocusedElementTag =
        winrt::unbox_value_or<int64_t>(senderDependencyObject.GetValue(FocusZoneLastFocusedElementTagProperty()), -1);
    if (lastFocusedElementTag == -1) {
      return;
    }

    const auto host = GetNativeUIManagerHost(viewManager->GetReactInstance());
    if (host == nullptr) {
      return;
    }
    ShadowNodeBase *pShadowNode = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(lastFocusedElementTag));
    if (pShadowNode == nullptr) {
      return;
    }
    auto lastFocusedElement = pShadowNode->GetView();
    if (lastFocusedElement != nullptr && e.NewFocusedElement() != lastFocusedElement) {
      e.TrySetNewFocusedElement(lastFocusedElement);
    }
  }
}

void OnFocusZoneLosingFocus(const winrt::IInspectable &sender, const winrt::LosingFocusEventArgs &e) {
  auto focusedElement = e.OldFocusedElement();
  if (!focusedElement) {
    return;
  }

  auto senderFrameworkElement = sender.as<xaml::FrameworkElement>();
  if (winrt::unbox_value<bool>(senderFrameworkElement.GetValue(IsMovingFocusOutOfFocusZoneProperty()))) {
    return;
  }
  auto focusedElementTag = react::uwp::GetTag(focusedElement);
  if (focusedElementTag != -1) {
    // We purposely don't store a reference to the element directly since it
    // would be a strong reference.
    senderFrameworkElement.SetValue(FocusZoneLastFocusedElementTagProperty(), winrt::box_value(focusedElementTag));
  }
}

void SetIsFocusZone(xaml::UIElement element, ViewManagerBase *viewManager, bool isFocusZone) {
  if (isFocusZone && element.XYFocusKeyboardNavigation() != xaml::Input::XYFocusKeyboardNavigationMode::Enabled) {
    element.TabFocusNavigation(xaml::Input::KeyboardNavigationMode::Once);
    element.XYFocusKeyboardNavigation(xaml::Input::XYFocusKeyboardNavigationMode::Enabled);
    element.KeyDown(OnFocusZoneKeyDown);
    element.GettingFocus([=](const winrt::IInspectable &sender, const winrt::GettingFocusEventArgs &e) {
      OnFocusZoneGettingFocus(sender, e, viewManager);
    });
    element.LosingFocus(OnFocusZoneLosingFocus);
  } else if (
      !isFocusZone && element.XYFocusKeyboardNavigation() == xaml::Input::XYFocusKeyboardNavigationMode::Enabled) {
    const auto instance = viewManager->GetReactInstance().lock();
    if (instance) {
      instance->CallJsFunction(
          "RCTLog",
          "logToConsole",
          folly::dynamic::array(
              "warn",
              "It's unsupported to set isFocusZone=false on a component which previously had isFocusZone=true."));
    }
  }
}

void UpdateFocusZoneXYFocusNavigationStrategy(winrt::UIElement focusZoneElement, const folly::dynamic &propertyValue) {
  auto xyFocusNavigationStrategy = json_type_traits<xaml::Input::XYFocusNavigationStrategy>::parseJson(propertyValue);
  focusZoneElement.XYFocusDownNavigationStrategy(xyFocusNavigationStrategy);
  focusZoneElement.XYFocusUpNavigationStrategy(xyFocusNavigationStrategy);
  focusZoneElement.XYFocusLeftNavigationStrategy(xyFocusNavigationStrategy);
  focusZoneElement.XYFocusRightNavigationStrategy(xyFocusNavigationStrategy);
}

} // namespace react::uwp
