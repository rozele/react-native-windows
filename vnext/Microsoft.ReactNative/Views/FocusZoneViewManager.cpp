// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"

#include "FocusZoneUtils.h"
#include "FocusZoneViewManager.h"

#include "ViewControl.h"

#include "DynamicAutomationProperties.h"

#include <Modules/NativeUIManager.h>
#include <Utils/AccessibilityUtils.h>
#include <Utils/PropertyUtils.h>

#include <INativeUIManager.h>
#include <IReactInstance.h>

#include <UI.Xaml.Controls.Primitives.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>

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
};

void FocusZoneViewShadowNode::createView() {
  Super::createView();

  auto panel = GetViewPanel();
  SetIsFocusZone(panel, GetViewManager(), true);
}

void FocusZoneViewShadowNode::updateProperties(const folly::dynamic &&props) {
  for (auto &pair : props.items()) {
    const std::string &propertyName = pair.first.getString();
    const folly::dynamic &propertyValue = pair.second;

    if (propertyName == "xyFocusNavigationStrategy") {
      UpdateFocusZoneXYFocusNavigationStrategy(GetViewPanel(), propertyValue);
    }
  }

  Super::updateProperties(std::move(props));
}

// FocusZoneViewManager

FocusZoneViewManager::FocusZoneViewManager(const std::shared_ptr<IReactInstance> &reactInstance)
    : Super(reactInstance) {}

const char *FocusZoneViewManager::GetName() const {
  return "FocusZoneView";
}

folly::dynamic FocusZoneViewManager::GetNativeProps() const {
  auto props = Super::GetNativeProps();

  props.update(folly::dynamic::object("xyFocusNavigationStrategy", "string"));

  return props;
}

facebook::react::ShadowNode *FocusZoneViewManager::createShadow() const {
  return new FocusZoneViewShadowNode();
}

} // namespace react::uwp
