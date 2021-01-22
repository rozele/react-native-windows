// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"

#include "FocusZoneViewManager.h"

#include "ViewControl.h"

#include "DynamicAutomationProperties.h"

#include <Modules/NativeUIManager.h>
#include <Utils/AccessibilityUtils.h>
#include <Utils/PropertyUtils.h>

#include <INativeUIManager.h>
#include <IReactInstance.h>

#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <UI.Xaml.Controls.Primitives.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>

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

namespace react::uwp {

class FocusZoneViewShadowNode : public ViewShadowNode {
  using Super = ViewShadowNode;
 public:
  FocusZoneViewShadowNode() = default;

  void createView() override;
  void updateProperties(const folly::dynamic &&props) override;
 private:
  void OnKeyDown(const winrt::IInspectable &sender, const winrt::KeyRoutedEventArgs &e);
  void OnGettingFocus(const winrt::IInspectable &sender, const winrt::GettingFocusEventArgs &e);
  void OnLosingFocus(const winrt::IInspectable &sender, const winrt::LosingFocusEventArgs &e);

  bool m_isHorizontal = false;
  winrt::weak_ref<xaml::DependencyObject> m_lastElement = nullptr;
  xaml::UIElement::KeyDown_revoker m_panelKeyDownRevoker{};
  xaml::UIElement::GettingFocus_revoker m_panelGettingFocusRevoker{};
  xaml::UIElement::LosingFocus_revoker m_panelLosingFocusRevoker{};
};

void FocusZoneViewShadowNode::createView() {
  Super::createView();

  auto panel = GetViewPanel();
  m_panelKeyDownRevoker = panel.KeyDown(winrt::auto_revoke, [=](const winrt::IInspectable &sender, const winrt::KeyRoutedEventArgs &e) {
    OnKeyDown(sender, e);
  });
  m_panelGettingFocusRevoker = panel.GettingFocus(winrt::auto_revoke, [=](const winrt::IInspectable &sender, const winrt::GettingFocusEventArgs &e) {
    OnGettingFocus(sender, e);
  });
  m_panelLosingFocusRevoker = panel.LosingFocus(winrt::auto_revoke, [=](const winrt::IInspectable &sender, const winrt::LosingFocusEventArgs &e) {
    OnLosingFocus(sender, e);
  });
}

void FocusZoneViewShadowNode::updateProperties(const folly::dynamic &&props) {
  for (auto &pair : props.items()) {
    const std::string &propertyName = pair.first.getString();
    const folly::dynamic &propertyValue = pair.second;

    if (propertyName == "focusDirection") {
      if (propertyValue.isString()) {
        m_isHorizontal = propertyValue.asString() == "horizontal";
      }
    }
  }

  Super::updateProperties(std::move(props));
}

void FocusZoneViewShadowNode::OnKeyDown(const winrt::IInspectable &sender, const winrt::KeyRoutedEventArgs &e) {
  auto senderFrameworkElement = sender.try_as<xaml::FrameworkElement>();
  auto xamlRoot = senderFrameworkElement.XamlRoot();
  auto focusedElement = winrt::FocusManager::GetFocusedElement(xamlRoot).try_as<xaml::DependencyObject>();
  winrt::FindNextElementOptions findNextElementOptions = winrt::FindNextElementOptions();
  findNextElementOptions.SearchRoot(xamlRoot.Content());
  auto previousKey = m_isHorizontal ? winrt::Windows::System::VirtualKey::Left : winrt::Windows::System::VirtualKey::Up;
  auto nextKey = m_isHorizontal ? winrt::Windows::System::VirtualKey::Right : winrt::Windows::System::VirtualKey::Down;

  if (e.Key() == previousKey) {
    auto firstFocusableElement =
        winrt::FocusManager::FindFirstFocusableElement(senderFrameworkElement).try_as<xaml::DependencyObject>();
    auto isFirstElementFocused = firstFocusableElement == focusedElement;
    if (!isFirstElementFocused)
    {
      winrt::FocusManager::TryMoveFocus(winrt::FocusNavigationDirection::Previous, findNextElementOptions);
      e.Handled(true);
    }
  } else if (e.Key() == nextKey) {
    auto lastFocusableElement = winrt::FocusManager::FindLastFocusableElement(senderFrameworkElement).try_as<xaml::DependencyObject>();
    auto isLastElementFocused = lastFocusableElement == focusedElement;
    if (!isLastElementFocused)
    {
      winrt::FocusManager::TryMoveFocus(winrt::FocusNavigationDirection::Next, findNextElementOptions);
      e.Handled(true);
    }
  } else if (e.Key() == winrt::Windows::System::VirtualKey::Tab) {
    auto const &coreWindow = winrt::CoreWindow::GetForCurrentThread();
    auto isShiftDown = KeyboardHelper::IsModifiedKeyPressed(coreWindow, winrt::Windows::System::VirtualKey::Shift);

    winrt::Point anchorTopLeft = winrt::Point(0, 0);
    winrt::GeneralTransform transform = senderFrameworkElement.TransformToVisual(xamlRoot.Content());
    winrt::Point anchorTopLeftConverted = transform.TransformPoint(anchorTopLeft);
    auto exclusionRect = winrt::Rect(anchorTopLeftConverted.X, anchorTopLeftConverted.Y, senderFrameworkElement.ActualWidth(), senderFrameworkElement.ActualHeight());
    findNextElementOptions.ExclusionRect(exclusionRect);

    auto nextElement = winrt::FocusManager::FindNextElement(isShiftDown ? winrt::FocusNavigationDirection::Up : winrt::FocusNavigationDirection::Down, findNextElementOptions);
    if (nextElement) {
      winrt::FocusManager::TryFocusAsync(
          nextElement, winrt::FocusState::Programmatic);
    }
    e.Handled(true);
  }
}

void FocusZoneViewShadowNode::OnGettingFocus(const winrt::IInspectable &sender, const winrt::GettingFocusEventArgs &e) {
  if (e.FocusState() != winrt::FocusState::Programmatic && m_lastElement.get() != nullptr && e.NewFocusedElement() != m_lastElement.get()) {
      e.TrySetNewFocusedElement(m_lastElement.get());
  }
}

void FocusZoneViewShadowNode::OnLosingFocus(const winrt::IInspectable &sender, const winrt::LosingFocusEventArgs &e) {
  m_lastElement = e.OldFocusedElement();
}

// FocusZoneViewManager

FocusZoneViewManager::FocusZoneViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

const char *FocusZoneViewManager::GetName() const {
  return "FocusZoneView";
}

folly::dynamic FocusZoneViewManager::GetNativeProps() const {
  auto props = Super::GetNativeProps();

  props.update(
      folly::dynamic::object("focusDirection", "string"));

  return props;
}

facebook::react::ShadowNode *FocusZoneViewManager::createShadow() const {
  return new FocusZoneViewShadowNode();
}

} // namespace react::uwp
