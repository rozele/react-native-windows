// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "WindowsTextInputComponentView.h"

#include <Fabric/WinUI/FabricKeyboardEventHandler.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Input.h>
#include <Utils/PropertyUtils.h>
#include <Utils/ResourceBrushUtils.h>
#include <Utils/ValueUtils.h>
#include <XamlView.h>
#include "Unicode.h"
#include "Utils/XamlIslandUtils.h"
#include "WindowsTextInputShadowNode.h"
#include "WindowsTextInputState.h"

void updateTextAlignment(
    xaml::Controls::TextBox &textBox,
    std::optional<facebook::react::TextAlignment> const &newAlignment) {
  if (newAlignment == facebook::react::TextAlignment::Right) {
    textBox.TextAlignment(xaml::TextAlignment::Right);
  } else if (newAlignment == facebook::react::TextAlignment::Left) {
    textBox.TextAlignment(xaml::TextAlignment::Left);
  } else if (newAlignment == facebook::react::TextAlignment::Center) {
    textBox.TextAlignment(xaml::TextAlignment::Center);
  } else if (newAlignment == facebook::react::TextAlignment::Justified) {
    textBox.TextAlignment(xaml::TextAlignment::Justify);
  } else {
    textBox.TextAlignment(xaml::TextAlignment::DetectFromContent);
  }
}

namespace Microsoft::ReactNative {

facebook::react::AttributedString WindowsTextInputComponentView::getAttributedString() const {
  // Use BaseTextShadowNode to get attributed string from children
  const auto &props = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(m_props);

  const auto textBox = m_control.try_as<xaml::Controls::TextBox>();
  auto childTextAttributes = facebook::react::TextAttributes::defaultTextAttributes();

  childTextAttributes.apply(props.textAttributes);

  auto attributedString = facebook::react::AttributedString{};
  // auto attachments = facebook::react::BaseTextShadowNode::Attachments{};

  // BaseTextShadowNode only gets children. We must detect and prepend text
  // value attributes manually.
  const auto text = to_string(textBox ? textBox.Text() : m_control.as<xaml::Controls::PasswordBox>().Password());
  if (!text.empty()) {
    auto textAttributes = facebook::react::TextAttributes::defaultTextAttributes();
    textAttributes.apply(props.textAttributes);
    auto fragment = facebook::react::AttributedString::Fragment{};
    fragment.string = text;
    fragment.textAttributes = textAttributes;
    fragment.textAttributes.backgroundColor = facebook::react::clearColor();
    attributedString.prependFragment(fragment);
  }

  return attributedString;
}

WindowsTextInputComponentView::WindowsTextInputComponentView(
    const winrt::Microsoft::ReactNative::ReactContext &context) {
  static auto const defaultProps = std::make_shared<facebook::react::WindowsTextInputProps const>();
  m_layoutMetrics = facebook::react::EmptyLayoutMetrics;
  m_context = context;
  m_props = defaultProps;
  registerEvents();
}

void WindowsTextInputComponentView::ReparentView(xaml::Controls::Control oldView) {
  if (const auto parent = oldView.Parent()) {
    const auto parentTag = static_cast<facebook::react::Tag>(GetTag(parent));
    if (const auto uiManager =
            FabricUIManager::FromProperties(winrt::Microsoft::ReactNative::ReactPropertyBag(m_context.Properties()))) {
      SetTag(m_control, GetTag(oldView));
      updateLayoutMetrics(m_layoutMetrics, facebook::react::EmptyLayoutMetrics);
      if (auto parentView = std::static_pointer_cast<BaseComponentView>(
              uiManager->GetViewRegistry().findComponentViewWithTag(parentTag))) {
        parentView->ReplaceChild(oldView, m_control);
      } else {
        assert(false);
      }
    }
  }
}

void WindowsTextInputComponentView::registerEvents() noexcept {
  if (const auto textBox = m_control.try_as<xaml::Controls::TextBox>()) {
    auto tag = GetTag(textBox);
    m_passwordBoxPasswordChangedRevoker = {};
    m_passwordBoxPasswordChangingRevoker = {};
    m_textChangingRevoker = textBox.TextChanging(winrt::auto_revoke, [this](const auto &sender, auto &&) {
      const auto textBox = sender.as<xaml::Controls::TextBox>();
      if (m_eventEmitter) {
        auto emitter = std::static_pointer_cast<const facebook::react::WindowsTextInputEventEmitter>(m_eventEmitter);
        facebook::react::WindowsTextInputMetrics textInputMetricsArgs;
        textInputMetricsArgs.text = winrt::to_string(textBox.Text());
        textInputMetricsArgs.eventCount = m_nativeEventCount++;
        emitter->onChange(textInputMetricsArgs);
      }
    });
    m_SelectionChangedRevoker = textBox.SelectionChanged(winrt::auto_revoke, [this](const auto &sender, auto args) {
      const auto textBox = sender.as<xaml::Controls::TextBox>();
      if (m_eventEmitter) {
        auto emitter = std::static_pointer_cast<const facebook::react::WindowsTextInputEventEmitter>(m_eventEmitter);
        facebook::react::WindowsTextInputEventEmitter::OnSelectionChange onSelectionChangeArgs;
        onSelectionChangeArgs.selection.start = textBox.SelectionStart();
        onSelectionChangeArgs.selection.end = textBox.SelectionStart() + textBox.SelectionLength();
        emitter->onSelectionChange(onSelectionChangeArgs);
      }
    });
  } else {
    m_textChangingRevoker = {};
    m_SelectionChangedRevoker = {};
    auto passwordBox = m_control.try_as<xaml::Controls::PasswordBox>();

    // IPasswordBox4 includes the APIs where PasswordChanging was introduced.
    // PasswordChanging is favored over PasswordChanged as it will not result in lost characters when typing fast
    if (passwordBox.try_as<xaml::Controls::IPasswordBox4>()) {
      m_passwordBoxPasswordChangingRevoker =
          passwordBox.PasswordChanging(winrt::auto_revoke, [this](const auto &sender, auto &&) {
            const auto passwordBox = m_control.as<xaml::Controls::PasswordBox>();
            if (m_eventEmitter) {
              auto emitter =
                  std::static_pointer_cast<const facebook::react::WindowsTextInputEventEmitter>(m_eventEmitter);
              facebook::react::WindowsTextInputMetrics textInputMetricsArgs;
              textInputMetricsArgs.text = winrt::to_string(passwordBox.Password());
              textInputMetricsArgs.eventCount = m_nativeEventCount++;
              emitter->onChange(textInputMetricsArgs);
            }
          });
    } else {
      m_passwordBoxPasswordChangedRevoker =
          passwordBox.PasswordChanged(winrt::auto_revoke, [this](const auto &sender, auto &&) {
            const auto passwordBox = m_control.as<xaml::Controls::PasswordBox>();
            if (m_eventEmitter) {
              auto emitter =
                  std::static_pointer_cast<const facebook::react::WindowsTextInputEventEmitter>(m_eventEmitter);
              facebook::react::WindowsTextInputMetrics textInputMetricsArgs;
              textInputMetricsArgs.text = winrt::to_string(passwordBox.Password());
              textInputMetricsArgs.eventCount = m_nativeEventCount++;
              emitter->onChange(textInputMetricsArgs);
            }
          });
    }
  }
  registerPreviewKeyDown();
}

void WindowsTextInputComponentView::registerPreviewKeyDown() noexcept {
  m_controlPreviewKeyDownRevoker = m_control.PreviewKeyDown(
      winrt::auto_revoke, [this](const auto &sender, xaml::Input::KeyRoutedEventArgs const &args) {
        const auto &props = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(m_props);
        auto shouldSubmit = !args.Handled();
        if (shouldSubmit) {
          if (!props.multiline && props.submitKeyEvents.size() == 0) {
            // For single line TextInput without custom submit keys, submit when user presses enter
            shouldSubmit = args.Key() == winrt::Windows::System::VirtualKey::Enter;
          } else if (props.submitKeyEvents.size() > 0) {
            // If custom submit keys are provided, use them to determine shouldSubmit
            auto defaultEventPhase = facebook::react::HandledEventPhase::Bubbling;
            auto currentEvent = FabricKeyboardHelper::CreateKeyboardEvent(defaultEventPhase, args);
            shouldSubmit = FabricKeyboardHelper::ShouldMarkKeyboardHandled(props.submitKeyEvents, currentEvent);
          } else {
            // If no custom submit keys are provided and multiline is enabled, disable submitting
            shouldSubmit = false;
          }
        }
        if (shouldSubmit && m_eventEmitter) {
          const auto textBox = sender.try_as<xaml::Controls::TextBox>();
          auto emitter = std::static_pointer_cast<const facebook::react::WindowsTextInputEventEmitter>(m_eventEmitter);
          facebook::react::WindowsTextInputMetrics textInputMetricsArgs;
          textInputMetricsArgs.text =
              winrt::to_string(textBox ? textBox.Text() : sender.as<xaml::Controls::PasswordBox>().Password());
          textInputMetricsArgs.eventCount = m_nativeEventCount;
          if (textBox) {
            textInputMetricsArgs.selectionRange.location = textBox.SelectionStart();
            textInputMetricsArgs.selectionRange.length = textBox.SelectionLength();
          }
          emitter->onSubmitEditing(textInputMetricsArgs);
          if (props.clearTextOnSubmit) {
            const auto textProperty =
                textBox ? xaml::Controls::TextBox::TextProperty() : xaml::Controls::PasswordBox::PasswordProperty();
            sender.as<xaml::Controls::Control>().ClearValue(textProperty);
          }
          if (props.multiline) {
            args.Handled(true);
          }
        }
      });
}

void WindowsTextInputComponentView::handleCommand(std::string const &commandName, folly::dynamic const &arg) noexcept {
  auto textBox = m_control.try_as<xaml::Controls::TextBox>();
  if (commandName == "setTextAndSelection") {
    auto eventCount = arg[0].asInt();

    if (eventCount >= m_nativeEventCount) {
      auto text = arg[1].asString();
      auto begin = arg[2].asInt();
      auto end = arg[3].asInt();
      m_comingFromJS = true;
      SetText(winrt::to_hstring(text));

      if (textBox && !(begin < 0 || end < 0 || begin > end)) {
        textBox.Select(static_cast<int32_t>(begin), static_cast<int32_t>(end - begin));
      }
      m_comingFromJS = false;
    }
  } else {
    Super::handleCommand(commandName, arg);
  }
}

std::vector<facebook::react::ComponentDescriptorProvider>
WindowsTextInputComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void WindowsTextInputComponentView::updatePropsTextBox(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(m_props);
  const auto &newTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(props);
  auto textBox = m_control.as<xaml::Controls::TextBox>();

  // Props shared between both PasswordBox and TextBox, but not shared with xaml::Controls::Control
  if (oldTextInputProps.maxLength != newTextInputProps.maxLength) {
    textBox.MaxLength(newTextInputProps.maxLength);
  }
  if (oldTextInputProps.selectionColor != newTextInputProps.selectionColor) {
    if (newTextInputProps.selectionColor) {
      textBox.SelectionHighlightColor(SolidBrushFromColor(newTextInputProps.selectionColor.AsWindowsColor()));
    } else {
      textBox.ClearValue(xaml::Controls::TextBox::SelectionHighlightColorProperty());
    }
  }
  if (oldTextInputProps.placeholder != newTextInputProps.placeholder) {
    textBox.PlaceholderText(winrt::to_hstring(newTextInputProps.placeholder));
  }
  if (oldTextInputProps.editable != newTextInputProps.editable) {
    textBox.IsReadOnly(!newTextInputProps.editable);
  }
  if (oldTextInputProps.placeholderTextColor != newTextInputProps.placeholderTextColor) {
    if (newTextInputProps.placeholderTextColor) {
      textBox.PlaceholderForeground(
          xaml::Media::SolidColorBrush(newTextInputProps.placeholderTextColor.AsWindowsColor()));
    } else {
      textBox.ClearValue(xaml::Controls::TextBox::PlaceholderForegroundProperty());
    }
  }

  // Props specific to TextBox
  if (oldTextInputProps.textAttributes.alignment != newTextInputProps.textAttributes.alignment) {
    if (newTextInputProps.textAttributes.alignment) {
      updateTextAlignment(textBox, newTextInputProps.textAttributes.alignment);
    } else {
      textBox.ClearValue(xaml::Controls::TextBox::TextAlignmentProperty());
    }
  }
  if (oldTextInputProps.multiline != newTextInputProps.multiline) {
    textBox.TextWrapping(newTextInputProps.multiline ? xaml::TextWrapping::Wrap : xaml::TextWrapping::NoWrap);
    textBox.AcceptsReturn(newTextInputProps.multiline);
  }
  if (oldTextInputProps.selection.start != newTextInputProps.selection.start ||
      oldTextInputProps.selection.end != newTextInputProps.selection.end) {
    textBox.Select(
        newTextInputProps.selection.start, newTextInputProps.selection.end - newTextInputProps.selection.start);
  }
  if (oldTextInputProps.autoCapitalize != newTextInputProps.autoCapitalize) {
    if (newTextInputProps.autoCapitalize == "characters") {
      textBox.CharacterCasing(xaml::Controls::CharacterCasing::Upper);
    } else { // anything else turns off autoCap (should be "None" but
             // we don't support "words"/"senetences" yet)
      textBox.CharacterCasing(xaml::Controls::CharacterCasing::Normal);
    }
  }
}

void WindowsTextInputComponentView::updatePropsPasswordBox(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(m_props);
  const auto &newTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(props);
  auto passwordBox = m_control.as<xaml::Controls::PasswordBox>();
  if (oldTextInputProps.maxLength != newTextInputProps.maxLength) {
    passwordBox.MaxLength(newTextInputProps.maxLength);
  }
  if (oldTextInputProps.selectionColor != newTextInputProps.selectionColor) {
    if (newTextInputProps.selectionColor) {
      passwordBox.SelectionHighlightColor(
          xaml::Media::SolidColorBrush(newTextInputProps.selectionColor.AsWindowsColor()));
    } else {
      passwordBox.ClearValue(xaml::Controls::PasswordBox::SelectionHighlightColorProperty());
    }
  }
  if (oldTextInputProps.placeholder != newTextInputProps.placeholder) {
    passwordBox.PlaceholderText(winrt::to_hstring(newTextInputProps.placeholder));
  }
  if (oldTextInputProps.editable != newTextInputProps.editable) {
    passwordBox.IsEnabled(newTextInputProps.editable);
  }
  if (oldTextInputProps.placeholderTextColor != newTextInputProps.placeholderTextColor) {
    // TODO(T145117327): Implement setPasswordBoxPlaceholderForeground
  }
}

void WindowsTextInputComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(m_props);
  const auto &newTextInputProps = *std::static_pointer_cast<const facebook::react::WindowsTextInputProps>(props);

  auto textBox = m_control.try_as<xaml::Controls::TextBox>();
  auto passwordBox = m_control.try_as<xaml::Controls::PasswordBox>();
  auto isTextBox = static_cast<bool>(textBox);

  if (oldTextInputProps.secureTextEntry != newTextInputProps.secureTextEntry) {
    xaml::Controls::Control newControl = xaml::Controls::TextBox();
    if (newTextInputProps.secureTextEntry)
      newControl = xaml::Controls::PasswordBox();
    isTextBox = !newTextInputProps.secureTextEntry;
    const auto oldControl = m_control;
    m_control = newControl;

    // Re-calling some functions to provide the new control with the same state, props, etc as the old control
    ReparentView(oldControl);
    registerEvents();

    // Resetting m_props as updateProps uses it instead of oldProps that's passed in
    // Setting secureTextEntry to  ensure m_props is in the same state as m_control with secureTextEntry
    // prop already processed
    auto defaultProps = facebook::react::WindowsTextInputProps();
    defaultProps.secureTextEntry = newTextInputProps.secureTextEntry;
    m_props = std::make_shared<facebook::react::WindowsTextInputProps const>(defaultProps);

    updateProps(props, {});
    registerPreviewKeyDown();

    if (newTextInputProps.secureTextEntry) {
      m_control.as<xaml::Controls::PasswordBox>().Password(textBox.Text());
    } else {
      m_control.as<xaml::Controls::TextBox>().Text(passwordBox.Password());
    }
  }

  if (oldTextInputProps.textAttributes.foregroundColor != newTextInputProps.textAttributes.foregroundColor) {
    if (newTextInputProps.textAttributes.foregroundColor) {
      const auto newColorBrush = newTextInputProps.textAttributes.foregroundColor.AsWindowsBrush();
      m_control.Foreground(newColorBrush);
      UpdateControlForegroundResourceBrushes(m_control, newColorBrush);
    } else {
      // TODO: T142202708 ForegroundColor does not update when reset to undefined
      m_control.ClearValue(xaml::Controls::Control::ForegroundProperty());
      UpdateControlForegroundResourceBrushes(m_control, nullptr);
    }
  }

  if (oldTextInputProps.textAttributes.fontSize != newTextInputProps.textAttributes.fontSize) {
    if (std::isnan(newTextInputProps.textAttributes.fontSize)) {
      m_control.FontSize(facebook::react::TextAttributes::defaultTextAttributes().fontSize);
    } else {
      m_control.FontSize(newTextInputProps.textAttributes.fontSize);
    }
  }

  if (oldTextInputProps.textAttributes.fontWeight != newTextInputProps.textAttributes.fontWeight) {
    m_control.FontWeight(winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(
        newTextInputProps.textAttributes.fontWeight.value_or(static_cast<facebook::react::FontWeight>(400)))});
  }

  if (oldTextInputProps.textAttributes.fontStyle != newTextInputProps.textAttributes.fontStyle) {
    switch (newTextInputProps.textAttributes.fontStyle.value_or(facebook::react::FontStyle::Normal)) {
      case facebook::react::FontStyle::Italic:
        m_control.FontStyle(winrt::Windows::UI::Text::FontStyle::Italic);
        break;
      case facebook::react::FontStyle::Normal:
        m_control.FontStyle(winrt::Windows::UI::Text::FontStyle::Normal);
        break;
      case facebook::react::FontStyle::Oblique:
        m_control.FontStyle(winrt::Windows::UI::Text::FontStyle::Oblique);
        break;
      default:
        assert(false);
    }
  }

  if (oldTextInputProps.textAttributes.fontFamily != newTextInputProps.textAttributes.fontFamily) {
    if (newTextInputProps.textAttributes.fontFamily.empty())
      m_control.FontFamily(xaml::Media::FontFamily(L"Segoe UI"));
    else
      m_control.FontFamily(xaml::Media::FontFamily(
          Microsoft::Common::Unicode::Utf8ToUtf16(newTextInputProps.textAttributes.fontFamily)));
  }

  if (oldTextInputProps.allowFontScaling != newTextInputProps.allowFontScaling) {
    m_control.IsTextScaleFactorEnabled(newTextInputProps.allowFontScaling);
  }
  if (oldTextInputProps.backgroundColor != newTextInputProps.backgroundColor) {
    if (newTextInputProps.backgroundColor) {
      const auto newBackgroundBrush = newTextInputProps.backgroundColor.AsWindowsBrush();
      m_control.Background(newBackgroundBrush);
      UpdateControlBackgroundResourceBrushes(m_control, newBackgroundBrush);
    } else {
      // TODO: T142203681 Background color does not update when reset to undefined
      m_control.ClearValue(xaml::Controls::Control::BackgroundProperty());
      UpdateControlBackgroundResourceBrushes(m_control, nullptr);
    }
  }

  if (oldTextInputProps.borderColors != newTextInputProps.borderColors) {
    if (newTextInputProps.borderColors.all) {
      m_control.BorderBrush(newTextInputProps.borderColors.all->AsWindowsBrush());
    } else {
      m_control.ClearValue(xaml::Controls::Control::BorderBrushProperty());
    }
  }

  if (isTextBox) {
    updatePropsTextBox(props, oldProps);
  } else {
    updatePropsPasswordBox(props, oldProps);
  }

  Super::updateProps(props, oldProps);
}

void WindowsTextInputComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {
  m_state = std::static_pointer_cast<facebook::react::WindowsTextInputShadowNode::ConcreteState const>(state);
  if (const auto textBox = m_control.try_as<xaml::Controls::TextBox>()) {
    if (!m_state) {
      assert(false && "State is `null` for <TextInput> component.");
      textBox.Text(L"");
      return;
    }
  }

  auto data = m_state->getData();

  if (!oldState) {
    m_mostRecentEventCount = m_state->getData().mostRecentEventCount;
  }

  if (m_mostRecentEventCount == m_state->getData().mostRecentEventCount) {
    m_comingFromState = true;
    // Only handle single/empty fragments right now -- ignore the other fragments
    SetText(
        m_state->getData().attributedString.getFragments().size()
            ? winrt::to_hstring(m_state->getData().attributedString.getFragments()[0].string)
            : L"");
    m_comingFromState = false;
  }
}

void WindowsTextInputComponentView::SetText(winrt::hstring text) noexcept {
  if (const auto textBox = m_control.try_as<xaml::Controls::TextBox>()) {
    auto oldCursor = textBox.SelectionStart();
    // TODO(T145950982): Update selection behavior to match Paper's
    auto oldSelectionLength = textBox.SelectionLength();
    auto oldValue = textBox.Text();
    auto newValue = text;
    if (oldValue != newValue) {
      textBox.Text(newValue);
      if (oldValue.size() == newValue.size()) {
        textBox.SelectionStart(oldCursor);
      } else {
        textBox.SelectionStart(newValue.size());
      }
    }

  } else {
    auto passwordBox = m_control.as<xaml::Controls::PasswordBox>();
    auto oldValue = passwordBox.Password();
    auto newValue = text;
    if (oldValue != newValue) {
      passwordBox.Password(newValue);
    }
  }
}

void WindowsTextInputComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  // Set Position & Size Properties

  m_control.BorderThickness(
      {layoutMetrics.borderWidth.left,
       layoutMetrics.borderWidth.top,
       layoutMetrics.borderWidth.right,
       layoutMetrics.borderWidth.bottom});

  // TODO(T142315946): Why is minHeight needed?
  m_control.MinHeight(0);

  m_control.Padding({
      layoutMetrics.contentInsets.left - layoutMetrics.borderWidth.left,
      layoutMetrics.contentInsets.top - layoutMetrics.borderWidth.top,
      layoutMetrics.contentInsets.right - layoutMetrics.borderWidth.right,
      layoutMetrics.contentInsets.bottom - layoutMetrics.borderWidth.bottom,
  });

  // TODO(T142315971): why is measurement required?
  const auto height = m_control.Height();
  const auto width = m_control.Width();
  m_control.ClearValue(xaml::FrameworkElement::HeightProperty());
  m_control.ClearValue(xaml::FrameworkElement::WidthProperty());
  m_control.Measure({std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()});
  auto ds = m_control.DesiredSize();
  m_control.Height(height);
  m_control.Width(width);

  Super::updateLayoutMetrics(layoutMetrics, oldLayoutMetrics);
  m_layoutMetrics = layoutMetrics;
}
void WindowsTextInputComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  // m_element.FinalizeProperties();
}
void WindowsTextInputComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement WindowsTextInputComponentView::Element() const noexcept {
  return m_control;
}

} // namespace Microsoft::ReactNative
