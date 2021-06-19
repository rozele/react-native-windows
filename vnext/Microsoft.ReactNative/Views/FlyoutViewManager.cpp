// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <Views/ShadowNodeBase.h>
#include "FlyoutViewManager.h"
#include "TouchEventHandler.h"
#include "ViewPanel.h"

#include <Modules/NativeUIManager.h>

// ARCHON_RNW_XAMLROOT Need to get xaml root for flyout
#include <ReactHost/UwpReactInstanceProxy.h>
#include <ReactHost/ReactInstanceWin.h>

#include <UI.Xaml.Controls.Primitives.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <Utils/Helpers.h>
#include <Utils/PropertyHandlerUtils.h>

#ifdef USE_WINUI3
namespace winrt::Microsoft::UI::Xaml::Controls::Primitives {
using IFlyoutBase5 = ::xaml::Controls::Primitives::IFlyoutBase;
using IFlyoutBase6 = ::xaml::Controls::Primitives::IFlyoutBase;
}; // namespace winrt::Microsoft::UI::Xaml::Controls::Primitives
#endif

namespace winrt {
using namespace xaml::Controls;
using namespace xaml::Controls::Primitives;
using namespace xaml::Interop;
using namespace winrt::Windows::UI::Xaml::Interop;
} // namespace winrt

static const std::unordered_map<std::string, winrt::FlyoutPlacementMode> placementModeMinVersion = {
    {"top", winrt::FlyoutPlacementMode::Top},
    {"top-edge-aligned-left", winrt::FlyoutPlacementMode::Top},
    {"top-edge-aligned-right", winrt::FlyoutPlacementMode::Top},
    {"bottom", winrt::FlyoutPlacementMode::Bottom},
    {"bottom-edge-aligned-left", winrt::FlyoutPlacementMode::Bottom},
    {"bottom-edge-aligned-right", winrt::FlyoutPlacementMode::Bottom},
    {"left", winrt::FlyoutPlacementMode::Left},
    {"left-edge-aligned-top", winrt::FlyoutPlacementMode::Left},
    {"left-edge-aligned-bottom", winrt::FlyoutPlacementMode::Left},
    {"right", winrt::FlyoutPlacementMode::Right},
    {"right-edge-aligned-top", winrt::FlyoutPlacementMode::Right},
    {"right-edge-aligned-bottom", winrt::FlyoutPlacementMode::Right},
    {"full", winrt::FlyoutPlacementMode::Full}};

static const std::unordered_map<std::string, winrt::FlyoutPlacementMode> placementModeRS5 = {
    {"top", winrt::FlyoutPlacementMode::Top},
    {"bottom", winrt::FlyoutPlacementMode::Bottom},
    {"left", winrt::FlyoutPlacementMode::Left},
    {"right", winrt::FlyoutPlacementMode::Right},
    {"full", winrt::FlyoutPlacementMode::Full},
    {"top-edge-aligned-left", winrt::FlyoutPlacementMode::TopEdgeAlignedLeft},
    {"top-edge-aligned-right", winrt::FlyoutPlacementMode::TopEdgeAlignedRight},
    {"bottom-edge-aligned-left", winrt::FlyoutPlacementMode::BottomEdgeAlignedLeft},
    {"bottom-edge-aligned-right", winrt::FlyoutPlacementMode::BottomEdgeAlignedRight},
    {"left-edge-aligned-top", winrt::FlyoutPlacementMode::LeftEdgeAlignedTop},
    {"left-edge-aligned-bottom", winrt::FlyoutPlacementMode::LeftEdgeAlignedBottom},
    {"right-edge-aligned-top", winrt::FlyoutPlacementMode::RightEdgeAlignedTop},
    {"right-edge-aligned-bottom", winrt::FlyoutPlacementMode::RightEdgeAlignedBottom}};

static const std::unordered_map<std::string, winrt::FlyoutShowMode> showMode = {
    {"auto", winrt::FlyoutShowMode::Auto},
    {"standard", winrt::FlyoutShowMode::Standard},
    {"transient", winrt::FlyoutShowMode::Transient},
    {"transient-with-dismiss-on-pointer-move-away", winrt::FlyoutShowMode::TransientWithDismissOnPointerMoveAway}};

template <>
struct json_type_traits<winrt::FlyoutPlacementMode> {
  static winrt::FlyoutPlacementMode parseJson(const folly::dynamic &json) {
    auto placementMode = !!(winrt::Flyout().try_as<winrt::IFlyoutBase5>()) ? placementModeRS5 : placementModeMinVersion;
    auto iter = placementMode.find(json.asString());

    if (iter != placementMode.end()) {
      return iter->second;
    }

    return winrt::FlyoutPlacementMode::Right;
  }
};

template <>
struct json_type_traits<winrt::FlyoutShowMode> {
  static winrt::FlyoutShowMode parseJson(const folly::dynamic &json) {
    auto iter = showMode.find(json.asString());

    if (iter != showMode.end()) {
      return iter->second;
    }

    return winrt::FlyoutShowMode::Auto;
  }
};

namespace react::uwp {

namespace FlyoutCommands {
constexpr const char *StopImmediateClosing = "stopImmediateClosing";
}; // namespace FlyoutCommands

class FlyoutShadowNode : public ShadowNodeBase {
  using Super = ShadowNodeBase;

 public:
  FlyoutShadowNode() = default;
  virtual ~FlyoutShadowNode();

  void dispatchCommand(const std::string &commandId, const folly::dynamic &commandArgs) override;
  void AddView(ShadowNode &child, int64_t index) override;
  void createView() override;
  static void OnFlyoutClosed(IReactInstance &instance, int64_t tag, bool newValue);
  void onDropViewInstance() override;
  void removeAllChildren() override;
  void updateProperties(const folly::dynamic &&props) override;
  winrt::Flyout GetFlyout();
  void AdjustDefaultFlyoutStyle(float maxWidth, float maxHeight);

  bool IsWindowed() override {
    return true;
  }

 private:
  void SetTargetFrameworkElement();
  winrt::Popup GetFlyoutParentPopup() const;
  winrt::FlyoutPresenter GetFlyoutPresenter() const;
  void OnShowFlyout();
  void LogErrorAndClose(const string &error);

  xaml::FrameworkElement m_targetElement = nullptr;
  winrt::Flyout m_flyout = nullptr;
  bool m_isLightDismissEnabled = true;
  bool m_isOpen = false;
  bool m_autoFocus = false;
  int64_t m_targetTag = -1;
  float m_horizontalOffset = 0;
  float m_verticalOffset = 0;
  bool m_isFlyoutShowOptionsSupported = false;
  winrt::FlyoutShowOptions m_showOptions = nullptr;
  winrt::FlyoutShowMode m_showMode = winrt::FlyoutShowMode::Auto;
  bool m_shouldStopImmediateClosing = false;

  std::unique_ptr<TouchEventHandler> m_touchEventHanadler;
  std::unique_ptr<PreviewKeyboardEventHandlerOnRoot> m_previewKeyboardEventHandlerOnRoot;

  winrt::Flyout::Closing_revoker m_flyoutClosingRevoker{};
  winrt::Flyout::Closed_revoker m_flyoutClosedRevoker{};
  int64_t m_tokenContentPropertyChangeCallback{0};
  winrt::Flyout::Opened_revoker m_flyoutOpenedRevoker{};
  winrt::XamlRoot::Changed_revoker m_xamlRootChangedRevoker{};
};

FlyoutShadowNode::~FlyoutShadowNode() {
  m_touchEventHanadler->RemoveTouchHandlers();
  m_previewKeyboardEventHandlerOnRoot->unhook();
}

void FlyoutShadowNode::dispatchCommand(const std::string &commandId, const folly::dynamic &commandArgs) {
  if (commandId == FlyoutCommands::StopImmediateClosing) {
    auto shouldStop = commandArgs[0].asBool();
    m_shouldStopImmediateClosing = shouldStop;
  }
}

void FlyoutShadowNode::AddView(ShadowNode &child, int64_t /*index*/) {
  auto childView = static_cast<ShadowNodeBase &>(child).GetView();
  m_touchEventHanadler->AddTouchHandlers(childView);
  m_previewKeyboardEventHandlerOnRoot->hook(childView);

  if (m_flyout != nullptr) {
    m_flyout.Content(childView.as<xaml::UIElement>());
    if (winrt::FlyoutPlacementMode::Full == m_flyout.Placement()) {
      // When using FlyoutPlacementMode::Full on a Flyout with an embedded
      // Picker, the flyout is not centered correctly. Below is a temporary
      // workaround that resolves the problem for flyouts with fixed size
      // content by adjusting the flyout presenter max size settings prior to
      // layout. This will unblock those scenarios while the work on a more
      // exhaustive fix proceeds.  Tracked by Issue #2969
      if (auto fe = m_flyout.Content().try_as<xaml::FrameworkElement>()) {
        AdjustDefaultFlyoutStyle((float)fe.Width(), (float)fe.Height());
      }
    }
  }
}

void FlyoutShadowNode::createView() {
  Super::createView();

  m_flyout = winrt::Flyout();
  m_flyout.ShouldConstrainToRootBounds(false);
  m_isFlyoutShowOptionsSupported = !!(winrt::Flyout().try_as<winrt::IFlyoutBase5>());

  if (m_isFlyoutShowOptionsSupported)
    m_showOptions = winrt::FlyoutShowOptions();

  auto wkinstance = GetViewManager()->GetReactInstance();
  m_touchEventHanadler = std::make_unique<TouchEventHandler>(wkinstance);
  m_previewKeyboardEventHandlerOnRoot = std::make_unique<PreviewKeyboardEventHandlerOnRoot>(wkinstance);

  m_flyoutClosingRevoker = m_flyout.Closing(
      winrt::auto_revoke, [=](winrt::FlyoutBase /*flyoutbase*/, winrt::FlyoutBaseClosingEventArgs args) {
        auto instance = wkinstance.lock();
        if (!m_updating && instance != nullptr && !m_isLightDismissEnabled && m_isOpen) {
          args.Cancel(true);
        }
        if (m_shouldStopImmediateClosing) {
          args.Cancel(true);
          m_shouldStopImmediateClosing = false;
        }
      });

  m_flyoutClosedRevoker = m_flyout.Closed(winrt::auto_revoke, [=](auto &&, auto &&) {
    auto instance = wkinstance.lock();
    if (!m_updating && instance != nullptr) {
      if (m_targetElement != nullptr) {
        // When the flyout closes, attempt to move focus to
        // its anchor element to prevent cases where focus can land on
        // an outer flyout content and therefore trigger a unexpected flyout
        // dismissal

        // ARCHON_RNW_MULTIWIN: If current XAML root does not match control
        // root, window does not have focus
        if (const auto reactInstance = static_cast<Mso::React::ReactInstanceWin*>(static_cast<UwpReactInstanceProxy*>(instance.get())->GetReactInstance().Get())) {
          if (m_targetElement.XamlRoot() == winrt::Microsoft::ReactNative::XamlUIService::GetXamlRoot(reactInstance->Options().Properties)) {
            xaml::Input::FocusManager::TryFocusAsync(
                m_targetElement, winrt::FocusState::Programmatic);
          }
        }
      }

      OnFlyoutClosed(*instance, m_tag, false);
      m_xamlRootChangedRevoker.revoke();
    }
  });

  // After opening the flyout, turn off AllowFocusOnInteraction so that
  // focus cannot land in the top content element of the flyout. If focus
  // lands in the flyout presenter, then on moving focus somewhere else,
  // including the children on the flyout presenter, the flyout can dismiss.
  // (Wait until after opening, so that the flyout presenter has a chance to
  // move focus to the first focusable element in the flyout.)
  //
  m_flyoutOpenedRevoker = m_flyout.Opened(winrt::auto_revoke, [=](auto &&, auto &&) {
    auto instance = wkinstance.lock();

    m_flyout.AllowFocusOnInteraction(false);

    if (!m_updating && instance != nullptr) {
      if (auto flyoutPresenter = GetFlyoutPresenter()) {
        // When multiple flyouts/popups are overlapping, XAML's theme
        // shadows don't render properly. As a workaround we enable a
        // z-index translation based on an elevation derived from the count
        // of open popups/flyouts. We apply this translation on open of the
        // flyout. (Translation is only supported on RS5+, eg. IUIElement9)
        if (auto uiElement9 = GetView().try_as<xaml::IUIElement9>()) {
          auto numOpenPopups = CountOpenPopups();
          if (numOpenPopups > 0) {
            winrt::Numerics::float3 translation{0, 0, (float)16 * numOpenPopups};
            flyoutPresenter.Translation(translation);
          }
        }

        if (m_autoFocus) {
          if (const auto content = m_flyout.Content()) {
            if (const auto elementToFocus = xaml::Input::FocusManager::FindFirstFocusableElement(content)) {
              if (const auto control = elementToFocus.try_as<xaml::Controls::Control>()) {
                if (const auto reactInstance = static_cast<Mso::React::ReactInstanceWin*>(static_cast<UwpReactInstanceProxy*>(instance.get())->GetReactInstance().Get())) {
                  if (control.XamlRoot() == winrt::Microsoft::ReactNative::XamlUIService::GetXamlRoot(reactInstance->Options().Properties)) {
                    control.Focus(xaml::FocusState::Programmatic);
                  }
                }
              }
            }
          }
        }

        flyoutPresenter.AllowFocusOnInteraction(false);
      }
    }
  });

  // Turning AllowFocusOnInteraction off at the root of the flyout and
  // flyout presenter turns it off for all children of the flyout. In order to
  // make sure that interactions with the content of the flyout still work as
  // expected, AllowFocusOnInteraction is turned on the content element when
  // the Content property is updated.
  m_tokenContentPropertyChangeCallback = m_flyout.RegisterPropertyChangedCallback(
      winrt::Flyout::ContentProperty(), [=](xaml::DependencyObject sender, xaml::DependencyProperty dp) {
        if (auto flyout = sender.try_as<winrt::Flyout>()) {
          if (auto content = flyout.Content()) {
            if (auto fe = content.try_as<xaml::FrameworkElement>()) {
              fe.AllowFocusOnInteraction(true);
            }
          }
        }
      });

  // Set XamlRoot on the Flyout to handle XamlIsland/AppWindow scenarios.
  if (auto flyoutBase6 = m_flyout.try_as<winrt::IFlyoutBase6>()) {
    if (auto instance = wkinstance.lock()) {
      // ARCHON_RNW_MULTIWIN: Get xaml root for flyout from the one we set on the context based on focus.
      // if (auto xamlRoot = static_cast<NativeUIManager *>(instance->NativeUIManager())->tryGetXamlRoot()) {
      auto riw = static_cast<Mso::React::ReactInstanceWin*>(static_cast<UwpReactInstanceProxy*>(instance.get())->GetReactInstance().Get());
      if (riw) {
        if (auto xamlRoot = winrt::Microsoft::ReactNative::XamlUIService::GetXamlRoot(riw->Options().Properties)) {
          flyoutBase6.XamlRoot(xamlRoot);
          m_xamlRootChangedRevoker = xamlRoot.Changed(winrt::auto_revoke, [this](auto &&, auto &&) {
            if (m_isLightDismissEnabled) {
              onDropViewInstance();
            }
          });
        }
      }
    }
  }
}

/*static*/ void FlyoutShadowNode::OnFlyoutClosed(IReactInstance &instance, int64_t tag, bool newValue) {
  folly::dynamic eventData = folly::dynamic::object("target", tag)("isOpen", newValue);
  instance.DispatchEvent(tag, "topDismiss", std::move(eventData));
}

void FlyoutShadowNode::onDropViewInstance() {
  if (m_isOpen) {
    m_isOpen = false;
    m_flyout.Hide();
  }
}

void FlyoutShadowNode::removeAllChildren() {
  m_flyout.ClearValue(winrt::Flyout::ContentProperty());
}

void FlyoutShadowNode::updateProperties(const folly::dynamic &&props) {
  m_updating = true;
  bool updateTargetElement = false;
  bool updateIsOpen = false;
  bool updateOffset = false;

  if (m_flyout == nullptr)
    return;

  for (auto &pair : props.items()) {
    const std::string &propertyName = pair.first.getString();
    const folly::dynamic &propertyValue = pair.second;

    if (propertyName == "horizontalOffset") {
      if (propertyValue.isNumber())
        m_horizontalOffset = static_cast<float>(propertyValue.asDouble());
      else
        m_horizontalOffset = 0;

      updateOffset = true;
    } else if (propertyName == "isLightDismissEnabled") {
      if (propertyValue.isBool())
        m_isLightDismissEnabled = propertyValue.asBool();
      else if (propertyValue.isNull())
        m_isLightDismissEnabled = true;
      if (m_isOpen) {
        auto popup = GetFlyoutParentPopup();
        if (popup != nullptr)
          popup.IsLightDismissEnabled(m_isLightDismissEnabled);
      }
    } else if (propertyName == "isOpen") {
      if (propertyValue.isBool()) {
        bool isOpen = m_isOpen;
        m_isOpen = propertyValue.asBool();
        if (isOpen != m_isOpen) {
          updateIsOpen = true;
        }
      }
    } else if (propertyName == "placement") {
      auto placement = json_type_traits<winrt::FlyoutPlacementMode>::parseJson(propertyValue);
      m_flyout.Placement(placement);
    } else if (propertyName == "target") {
      if (propertyValue.isNumber()) {
        m_targetTag = static_cast<int64_t>(propertyValue.asDouble());
        updateTargetElement = true;
      } else
        m_targetTag = -1;
    } else if (propertyName == "verticalOffset") {
      if (propertyValue.isNumber())
        m_verticalOffset = static_cast<float>(propertyValue.asDouble());
      else
        m_verticalOffset = 0;

      updateOffset = true;
    } else if (propertyName == "isOverlayEnabled") {
      auto overlayMode = winrt::LightDismissOverlayMode::Off;
      if (propertyValue.isBool() && propertyValue.asBool()) {
        overlayMode = winrt::LightDismissOverlayMode::On;
      }

      m_flyout.LightDismissOverlayMode(overlayMode);
    } else if (propertyName == "shouldConstrainToRootBounds") {
      if (propertyValue.isBool()) {
        m_flyout.ShouldConstrainToRootBounds(propertyValue.asBool());
      }
    } else if (propertyName == "showMode") {
      m_showMode = json_type_traits<winrt::FlyoutShowMode>::parseJson(propertyValue);
      m_flyout.ShowMode(m_showMode);
      if (m_isFlyoutShowOptionsSupported) {
        m_showOptions.ShowMode(m_showMode);
      }
    } else if (propertyName == "autoFocus") {
      if (propertyValue.isBool())
        m_autoFocus = propertyValue.asBool();
      else if (propertyValue.isNull())
        m_autoFocus = false;
    }
  }

  if (updateTargetElement || m_targetElement == nullptr) {
    SetTargetFrameworkElement();
    winrt::FlyoutBase::SetAttachedFlyout(m_targetElement, m_flyout);
  }

  if (updateOffset && m_isFlyoutShowOptionsSupported) {
    winrt::Point newPoint(m_horizontalOffset, m_verticalOffset);
    m_showOptions.Position(newPoint);
  }

  if (updateIsOpen) {
    if (m_isOpen) {
      OnShowFlyout();
    } else {
      m_flyout.Hide();
    }
  }

  // TODO: hook up view props to the flyout (m_flyout) instead of setting them
  // on the dummy view.
  // Super::updateProperties(std::move(props));
  m_updating = false;
}

winrt::Flyout FlyoutShadowNode::GetFlyout() {
  return m_flyout;
}

void FlyoutShadowNode::OnShowFlyout() {
  AdjustDefaultFlyoutStyle(50000, 50000);
  if (m_isFlyoutShowOptionsSupported) {
    if (m_showMode == winrt::FlyoutShowMode::Transient ||
        m_showMode == winrt::FlyoutShowMode::TransientWithDismissOnPointerMoveAway) {
      auto parent = winrt::VisualTreeHelper::GetParent(m_targetElement);
      if (parent == nullptr) {
        m_flyout.OverlayInputPassThroughElement(m_targetElement);
      } else {
        auto current = parent;
        while (parent != nullptr) {
          current = parent;
          parent = winrt::VisualTreeHelper::GetParent(current);
        }
        m_flyout.OverlayInputPassThroughElement(current);
      }
    }

    if (!m_targetElement && m_targetTag > 0) {
      LogErrorAndClose("The target view unmounted before flyout could be shown.");
    } else if (m_targetElement && m_flyout.XamlRoot() != m_targetElement.XamlRoot()) {
      LogErrorAndClose("The target view window lost focus before flyout could be shown.");
    } else {
      m_flyout.ShowAt(m_targetElement, m_showOptions);
    }
  } else {
    winrt::FlyoutBase::ShowAttachedFlyout(m_targetElement);
  }

  if (auto popup = GetFlyoutParentPopup()) {
    popup.IsLightDismissEnabled(m_isLightDismissEnabled);
  }
}

void FlyoutShadowNode::LogErrorAndClose(const std::string &error) {
  if (const auto instance = GetViewManager()->GetReactInstance().lock()) {
    instance->CallJsFunction("RCTLog", "logToConsole", folly::dynamic::array("error", error));
    OnFlyoutClosed(*instance, m_tag, false);
  }
}

void FlyoutShadowNode::SetTargetFrameworkElement() {
  const auto host = GetNativeUIManagerHost(GetViewManager()->GetReactInstance());
  if (host == nullptr)
    return;

  if (m_targetTag > 0) {
    ShadowNodeBase *pShadowNodeChild =
        static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(m_targetTag));

    if (pShadowNodeChild != nullptr) {
      auto targetView = pShadowNodeChild->GetView();
      m_targetElement = targetView.as<xaml::FrameworkElement>();
    }
  } else {
    if (auto flyoutBase6 = m_flyout.try_as<winrt::IFlyoutBase6>()) {
      m_targetElement = flyoutBase6.XamlRoot().Content().as<winrt::FrameworkElement>();
    }
  }
}

void FlyoutShadowNode::AdjustDefaultFlyoutStyle(float maxWidth, float maxHeight) {
  winrt::Style flyoutStyle({XAML_NAMESPACE_STR L".Controls.FlyoutPresenter", winrt::TypeKind::Metadata});
  flyoutStyle.Setters().Append(winrt::Setter(xaml::FrameworkElement::MaxWidthProperty(), winrt::box_value(maxWidth)));
  flyoutStyle.Setters().Append(winrt::Setter(xaml::FrameworkElement::MaxHeightProperty(), winrt::box_value(maxHeight)));
  flyoutStyle.Setters().Append(winrt::Setter(xaml::Controls::Control::PaddingProperty(), winrt::box_value(0)));
  flyoutStyle.Setters().Append(winrt::Setter(xaml::Controls::Control::BorderThicknessProperty(), winrt::box_value(0)));
  flyoutStyle.Setters().Append(
      winrt::Setter(xaml::FrameworkElement::AllowFocusOnInteractionProperty(), winrt::box_value(false)));
  flyoutStyle.Setters().Append(winrt::Setter(
      xaml::Controls::Control::BackgroundProperty(),
      winrt::box_value(xaml::Media::SolidColorBrush{winrt::Colors::Transparent()})));
  m_flyout.FlyoutPresenterStyle(flyoutStyle);
}

winrt::Popup FlyoutShadowNode::GetFlyoutParentPopup() const {
  // TODO: Use VisualTreeHelper::GetOpenPopupsFromXamlRoot when running against
  // RS6
  winrt::Windows::Foundation::Collections::IVectorView<winrt::Popup> popups =
      winrt::VisualTreeHelper::GetOpenPopups(xaml::Window::Current());
  if (popups.Size() > 0)
    return popups.GetAt(0);
  return nullptr;
}

winrt::FlyoutPresenter FlyoutShadowNode::GetFlyoutPresenter() const {
  if (m_flyout == nullptr || m_flyout.Content() == nullptr)
    return nullptr;

  auto parent = winrt::VisualTreeHelper::GetParent(m_flyout.Content());
  if (parent == nullptr)
    return nullptr;

  auto scope = parent.try_as<winrt::FlyoutPresenter>();
  while (parent != nullptr && scope == nullptr) {
    parent = winrt::VisualTreeHelper::GetParent(parent);
    scope = parent.try_as<winrt::FlyoutPresenter>();
  }

  return scope;
}

FlyoutViewManager::FlyoutViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

const char *FlyoutViewManager::GetName() const {
  return "RCTFlyout";
}

XamlView FlyoutViewManager::CreateViewCore(int64_t /*tag*/) {
  return winrt::make<winrt::react::uwp::implementation::ViewPanel>().as<XamlView>();
}

facebook::react::ShadowNode *FlyoutViewManager::createShadow() const {
  return new FlyoutShadowNode();
}

folly::dynamic FlyoutViewManager::GetCommands() const {
  return folly::dynamic::object(FlyoutCommands::StopImmediateClosing, FlyoutCommands::StopImmediateClosing);
}

folly::dynamic FlyoutViewManager::GetNativeProps() const {
  auto props = Super::GetNativeProps();

  props.update(folly::dynamic::object("horizontalOffset", "number")("isLightDismissEnabled", "boolean")(
      "isOpen", "boolean")("placement", "number")("showMode", "number")("target", "number")("verticalOffset", "number")(
      "isOverlayEnabled", "boolean")("shouldConstrainToRootBounds", "boolean")("autoFocus", "boolean"));

  return props;
}

folly::dynamic FlyoutViewManager::GetExportedCustomDirectEventTypeConstants() const {
  auto directEvents = Super::GetExportedCustomDirectEventTypeConstants();
  directEvents["topDismiss"] = folly::dynamic::object("registrationName", "onDismiss");

  return directEvents;
}

void FlyoutViewManager::SetLayoutProps(
    ShadowNodeBase &nodeToUpdate,
    const XamlView & /*viewToUpdate*/,
    float /*left*/,
    float /*top*/,
    float width,
    float height) {
  auto *pFlyoutShadowNode = static_cast<FlyoutShadowNode *>(&nodeToUpdate);

  if (auto flyout = pFlyoutShadowNode->GetFlyout()) {
    if (winrt::FlyoutPlacementMode::Full == flyout.Placement()) {
      pFlyoutShadowNode->AdjustDefaultFlyoutStyle(width, height);
    }
  }
}

} // namespace react::uwp
