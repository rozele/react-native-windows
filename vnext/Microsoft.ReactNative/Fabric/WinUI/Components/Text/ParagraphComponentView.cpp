// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ParagraphComponentView.h"

#include <IReactContext.h>
#include <UI.Input.h>
#include <UI.Text.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <Utils/TransformableText.h>
#include <Utils/ValueUtils.h>
#include <Utils/XamlIslandUtils.h>
#include <Views/Text/TextHitTestUtils.h>
#include <dwrite.h>
#include <react/renderer/components/text/ParagraphShadowNode.h>
#include <react/renderer/components/text/ParagraphState.h>
#include <unicode.h>
#include "XamlView.h"

namespace Microsoft::ReactNative {

template <class T>
void SetTextDecorations(
    T const &element,
    std::optional<facebook::react::TextDecorationLineType> const &textDecorationLineType) {
  using facebook::react::TextDecorationLineType;
  using text::TextDecorations;

  // TODO: Do we need an API version check here?
  if (textDecorationLineType.has_value()) {
    switch (textDecorationLineType.value()) {
      case TextDecorationLineType::Underline:
        element.TextDecorations(TextDecorations::Underline);
        break;
      case TextDecorationLineType::Strikethrough:
        element.TextDecorations(TextDecorations::Strikethrough);
        break;
      case TextDecorationLineType::UnderlineStrikethrough:
        element.TextDecorations(TextDecorations::Underline | TextDecorations::Strikethrough);
        break;
      case facebook::react::TextDecorationLineType::None:
        element.TextDecorations(TextDecorations::None);
        break;
      default:
        assert(false);
    }
  } else {
    element.ClearValue(T::TextDecorationsProperty());
  }
}

template <class T>
void SetFontStyle(T const &element, std::optional<facebook::react::FontStyle> const &fontStyle) {
  if (fontStyle.has_value()) {
    switch (fontStyle.value()) {
      case facebook::react::FontStyle::Italic:
        element.FontStyle(text::FontStyle::Italic);
        break;
      case facebook::react::FontStyle::Normal:
        element.FontStyle(text::FontStyle::Normal);
        break;
      case facebook::react::FontStyle::Oblique:
        element.FontStyle(text::FontStyle::Oblique);
        break;
      default:
        assert(false);
    }
  } else {
    element.ClearValue(T::FontStyleProperty());
  }
}

ParagraphComponentView::ParagraphComponentView(winrt::Microsoft::ReactNative::ReactContext const &reactContext)
    : m_context(reactContext) {
  static auto const defaultProps = std::make_shared<facebook::react::ParagraphProps const>();
  m_props = defaultProps;

  m_element.TextWrapping(xaml::TextWrapping::Wrap); // Default behavior in React Native
}

void ParagraphComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(props);

  auto updatedFontProperties = false;

  if (oldViewProps.textAttributes.foregroundColor != newViewProps.textAttributes.foregroundColor) {
    updatedFontProperties = true;
    if (newViewProps.textAttributes.foregroundColor) {
      m_element.Foreground(newViewProps.textAttributes.foregroundColor.AsWindowsBrush());
    } else {
      m_element.ClearValue(::xaml::Controls::TextBlock::ForegroundProperty());
    }
  }

  if (oldViewProps.textAttributes.fontSize != newViewProps.textAttributes.fontSize) {
    updatedFontProperties = true;
    if (std::isnan(newViewProps.textAttributes.fontSize)) {
      m_element.ClearValue(::xaml::Controls::TextBlock::FontSizeProperty());
    } else {
      m_element.FontSize(newViewProps.textAttributes.fontSize);
    }
  }

  if (oldViewProps.textAttributes.fontWeight != newViewProps.textAttributes.fontWeight) {
    updatedFontProperties = true;
    if (newViewProps.textAttributes.fontWeight.has_value()) {
      m_element.FontWeight(text::FontWeight{static_cast<uint16_t>(newViewProps.textAttributes.fontWeight.value())});
    } else {
      m_element.ClearValue(::xaml::Controls::TextBlock::FontWeightProperty());
    }
  }

  if (oldViewProps.textAttributes.fontStyle != newViewProps.textAttributes.fontStyle) {
    updatedFontProperties = true;
    SetFontStyle(m_element, newViewProps.textAttributes.fontStyle);
  }

  if (oldViewProps.textAttributes.textDecorationLineType != newViewProps.textAttributes.textDecorationLineType) {
    // TODO: Do we need an API version check here?
    updatedFontProperties = true;
    SetTextDecorations(m_element, newViewProps.textAttributes.textDecorationLineType);
  }

  if (oldViewProps.textAttributes.fontFamily != newViewProps.textAttributes.fontFamily) {
    updatedFontProperties = true;
    if (!newViewProps.textAttributes.fontFamily.empty()) {
      m_element.FontFamily(
          xaml::Media::FontFamily(Microsoft::Common::Unicode::Utf8ToUtf16(newViewProps.textAttributes.fontFamily)));
    } else {
      m_element.ClearValue(xaml::Controls::TextBlock::FontFamilyProperty());
    }
  }

  auto updatedBackgroundColor = oldViewProps.backgroundColor != newViewProps.backgroundColor;

  if (oldViewProps.isSelectable != newViewProps.isSelectable) {
    m_element.IsTextSelectionEnabled(newViewProps.isSelectable);
    ToggleTouchEvents(newViewProps.isSelectable);
  }

  if (oldViewProps.textAttributes.alignment != newViewProps.textAttributes.alignment) {
    if (newViewProps.textAttributes.alignment) {
      auto alignment = xaml::TextAlignment::DetectFromContent;
      switch (*newViewProps.textAttributes.alignment) {
        case facebook::react::TextAlignment::Center:
          alignment = xaml::TextAlignment::Center;
          break;
        case facebook::react::TextAlignment::Justified:
          alignment = xaml::TextAlignment::Justify;
          break;
        case facebook::react::TextAlignment::Left:
          alignment = xaml::TextAlignment::Left;
          break;
        case facebook::react::TextAlignment::Right:
          alignment = xaml::TextAlignment::Right;
          break;
        case facebook::react::TextAlignment::Natural:
          alignment = xaml::TextAlignment::Start;
          break;
        default:
          break;
      }
      m_element.TextAlignment(alignment);
    } else {
      m_element.ClearValue(xaml::Controls::TextBlock::TextAlignmentProperty());
    }
  }

  if (oldViewProps.textAttributes.lineHeight != newViewProps.textAttributes.lineHeight) {
    if (!isnan(newViewProps.textAttributes.lineHeight)) {
      m_element.LineStackingStrategy(xaml::LineStackingStrategy::BlockLineHeight);
      m_element.LineHeight(newViewProps.textAttributes.lineHeight);
    } else {
      m_element.ClearValue(xaml::Controls::TextBlock::LineHeightProperty());
      m_element.ClearValue(xaml::Controls::TextBlock::LineStackingStrategyProperty());
    }
  }

  if (oldViewProps.paragraphAttributes.ellipsizeMode != newViewProps.paragraphAttributes.ellipsizeMode) {
    if (newViewProps.paragraphAttributes.ellipsizeMode != facebook::react::EllipsizeMode::Clip) {
      m_element.TextTrimming(xaml::TextTrimming::CharacterEllipsis);
    } else {
      m_element.ClearValue(xaml::Controls::TextBlock::TextTrimmingProperty());
    }
  }

  Super::updateProps(props, oldProps);

  // When text is optimized and font properties are updated, we need to rebuild
  // the text from state in case the parent paragraph font properties no longer
  // match the inline text font properties. When background color is reset, we
  // need to unconditionally rebuild the text from state.
  if (m_state && (updatedBackgroundColor || (m_isOptimizedText && updatedFontProperties))) {
    m_rebuildText = true;
  }
}

const facebook::react::SharedViewEventEmitter &ParagraphComponentView::GetEventEmitter(
    facebook::react::Tag tag) const noexcept {
  const auto fragmentEventEmitterIter = m_fragmentEventEmitters.find(tag);
  if (fragmentEventEmitterIter != m_fragmentEventEmitters.end()) {
    return fragmentEventEmitterIter->second;
  }

  return Super::GetEventEmitter(tag);
}

// TODO(T142878585): DRY this helper function with the version defined in TextLayoutManager
Microsoft::ReactNative::TextTransform ConvertTextTransform(
    std::optional<facebook::react::TextTransform> const &transform) {
  if (transform) {
    switch (transform.value()) {
      case facebook::react::TextTransform::Capitalize:
        return Microsoft::ReactNative::TextTransform::Capitalize;
      case facebook::react::TextTransform::Lowercase:
        return Microsoft::ReactNative::TextTransform::Lowercase;
      case facebook::react::TextTransform::Uppercase:
        return Microsoft::ReactNative::TextTransform::Uppercase;
      case facebook::react::TextTransform::None:
        return Microsoft::ReactNative::TextTransform::None;
      default:
        break;
    }
  }

  return Microsoft::ReactNative::TextTransform::Undefined;
}

void ParagraphComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {
  m_state = std::static_pointer_cast<facebook::react::ParagraphShadowNode::ConcreteState const>(state);
  const auto &newState = *m_state;
  const auto &attributedString = newState.getData().attributedString;
  if (oldState) {
    const auto &oldTextState =
        *std::static_pointer_cast<facebook::react::ParagraphShadowNode::ConcreteState const>(oldState);
    const auto &oldAttributedString = oldTextState.getData().attributedString;
    if (attributedString.isContentEqual(oldAttributedString)) {
      return;
    }
  }

  m_rebuildText = true;
}

void ParagraphComponentView::RebuildTextFromState() noexcept {
  const auto &attributedString = m_state->getData().attributedString;
  const auto &viewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(m_props);

  // Clear properties that may have been set for optimized text
  if (m_isOptimizedText) {
    m_element.ClearValue(xaml::Controls::TextBlock::TextProperty());
    if (viewProps.textAttributes.fontFamily.empty()) {
      m_element.ClearValue(xaml::Controls::TextBlock::FontFamilyProperty());
    }
    if (!viewProps.textAttributes.fontWeight.has_value()) {
      m_element.ClearValue(xaml::Controls::TextBlock::FontWeightProperty());
    }
    if (std::isnan(viewProps.textAttributes.fontSize)) {
      m_element.ClearValue(xaml::Controls::TextBlock::FontSizeProperty());
    }
    if (!viewProps.textAttributes.foregroundColor) {
      m_element.ClearValue(xaml::Controls::TextBlock::ForegroundProperty());
    }
    if (!viewProps.textAttributes.fontStyle.has_value()) {
      m_element.ClearValue(xaml::Controls::TextBlock::FontStyleProperty());
    }
    if (!viewProps.textAttributes.textDecorationLineType.has_value()) {
      m_element.ClearValue(xaml::Controls::TextBlock::TextDecorationsProperty());
    }
  } else {
    m_element.Inlines().Clear();
  }

  m_element.TextHighlighters().Clear();
  m_fragmentEventEmitters.clear();

  int32_t position = 0;
  const auto defaultBackground = facebook::react::isColorMeaningful(viewProps.backgroundColor)
      ? viewProps.backgroundColor.AsWindowsBrush()
      : nullptr;

  auto initialFragment = true;
  auto canOptimize = true;
  auto commonFontFamily = viewProps.textAttributes.fontFamily;
  auto commonFontWeight = viewProps.textAttributes.fontWeight;
  auto commonFontSize = viewProps.textAttributes.fontSize;
  auto commonForegroundColor = viewProps.textAttributes.foregroundColor;
  auto commonFontStyle = viewProps.textAttributes.fontStyle;
  auto commonTextDecorationLineType = viewProps.textAttributes.textDecorationLineType;

  winrt::hstring transformedText = L"";
  for (const auto &fragment : attributedString.getFragments()) {
    auto inlines = m_element.Inlines();
    const auto tag = fragment.parentShadowView.tag;

    xaml::Documents::Span span{nullptr};
    if (fragment.textAttributes.accessibilityRole == facebook::react::AccessibilityRole::Link) {
      span = CreateHyperlink(tag);
      inlines.Append(span);
      inlines = span.Inlines();
      canOptimize = false;
    }

    if (fragment.textAttributes.textDecorationLineType.has_value()) {
      if (!span) {
        span = xaml::Documents::Span{};
        inlines.Append(span);
        inlines = span.Inlines();
      }

      SetTextDecorations<xaml::Documents::TextElement>(span, fragment.textAttributes.textDecorationLineType);
      if (initialFragment && !commonTextDecorationLineType.has_value()) {
        commonTextDecorationLineType = fragment.textAttributes.textDecorationLineType;
      } else if (fragment.textAttributes.textDecorationLineType != commonTextDecorationLineType) {
        canOptimize = false;
      }
    }

    if (fragment.textAttributes.fontStyle.has_value()) {
      if (!span) {
        span = xaml::Documents::Span{};
        inlines.Append(span);
        inlines = span.Inlines();
      }

      SetFontStyle<xaml::Documents::TextElement>(span, fragment.textAttributes.fontStyle);
      if (initialFragment && !commonFontStyle.has_value()) {
        commonFontStyle = fragment.textAttributes.fontStyle;
      } else if (fragment.textAttributes.fontStyle != commonFontStyle) {
        canOptimize = false;
      }
    }

    const auto run = xaml::Documents::Run();
    if (fragment.textAttributes.isPressable.has_value() && *fragment.textAttributes.isPressable &&
        fragment.parentShadowView.eventEmitter) {
      SetTag(run, tag);
      m_fragmentEventEmitters[tag] =
          std::static_pointer_cast<facebook::react::ViewEventEmitter const>(fragment.parentShadowView.eventEmitter);
      canOptimize = false;
    }

    const auto fragmentText = TransformableText::TransformText(
        winrt::to_hstring(fragment.string), ConvertTextTransform(fragment.textAttributes.textTransform));
    run.Text(fragmentText);
    if (canOptimize) {
      transformedText = transformedText + fragmentText;
    }

    if (!fragment.textAttributes.fontFamily.empty()) {
      run.FontFamily(xaml::Media::FontFamily(winrt::to_hstring(fragment.textAttributes.fontFamily)));
      if (initialFragment && commonFontFamily.empty()) {
        commonFontFamily = fragment.textAttributes.fontFamily;
      } else if (fragment.textAttributes.fontFamily != commonFontFamily) {
        canOptimize = false;
      }
    }

    if (fragment.textAttributes.fontWeight.has_value()) {
      run.FontWeight(
          winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(fragment.textAttributes.fontWeight.value())});
      if (initialFragment && !commonFontWeight.has_value()) {
        commonFontWeight = fragment.textAttributes.fontWeight;
      } else if (fragment.textAttributes.fontWeight != commonFontWeight) {
        canOptimize = false;
      }
    }

    if (!std::isnan(fragment.textAttributes.fontSize)) {
      run.FontSize(fragment.textAttributes.fontSize);
      if (initialFragment && std::isnan(commonFontSize)) {
        commonFontSize = fragment.textAttributes.fontSize;
      } else if (fragment.textAttributes.fontSize != commonFontSize) {
        canOptimize = false;
      }
    }

    const auto foreground = fragment.textAttributes.foregroundColor.AsWindowsBrush();
    if (foreground) {
      run.Foreground(foreground);
      if (initialFragment && !commonForegroundColor) {
        commonForegroundColor = fragment.textAttributes.foregroundColor;
      } else if (fragment.textAttributes.foregroundColor != commonForegroundColor) {
        canOptimize = false;
      }
    }

    inlines.Append(run);

    // We need to track if the background is inherited from the paragraph in
    // case the backgroundColor prop changes. To simplify the logic for
    // updating the paragraph backgroundColor prop, we ensure that there is a
    // one-to-one mapping between fragments and highlighters.
    const auto hasNestedBackground = facebook::react::isColorMeaningful(fragment.textAttributes.backgroundColor);
    // If the default background brush is null, convert the non-meaningful
    // (i.e., transparent) backgroundColor prop to a transparent brush, since
    // a null background brush on a TextHighlighter produces a yellow highlight.
    const auto background = hasNestedBackground || !defaultBackground
        ? fragment.textAttributes.backgroundColor.AsWindowsBrush()
        : defaultBackground;

    const auto length = static_cast<int32_t>(run.Text().size());
    xaml::Documents::TextHighlighter highlighter;
    highlighter.Background(background);
    highlighter.Foreground(foreground);
    highlighter.Ranges().Append({position, length});
    m_element.TextHighlighters().Append(highlighter);

    position += length;
    initialFragment = false;
  }

  if (canOptimize) {
    m_element.Inlines().Clear();
    if (!commonFontFamily.empty() && viewProps.textAttributes.fontFamily.empty()) {
      m_element.FontFamily(xaml::Media::FontFamily(winrt::to_hstring(commonFontFamily)));
    }
    if (commonFontWeight.has_value() && !viewProps.textAttributes.fontWeight.has_value()) {
      m_element.FontWeight(winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(commonFontWeight.value())});
    }
    if (!std::isnan(commonFontSize) && std::isnan(viewProps.textAttributes.fontSize)) {
      m_element.FontSize(commonFontSize);
    }
    if (commonForegroundColor && !viewProps.textAttributes.foregroundColor) {
      m_element.Foreground(commonForegroundColor.AsWindowsBrush());
    }
    if (commonFontStyle.has_value() && !viewProps.textAttributes.fontStyle) {
      SetFontStyle(m_element, commonFontStyle);
    }
    if (commonTextDecorationLineType.has_value() && !viewProps.textAttributes.textDecorationLineType) {
      SetTextDecorations(m_element, commonTextDecorationLineType);
    }
    m_element.Text(transformedText);
  }
}

void ParagraphComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {
  if (m_rebuildText) {
    RebuildTextFromState();
    m_rebuildText = false;
  }
}

void ParagraphComponentView::prepareForRecycle() noexcept {}

const xaml::FrameworkElement ParagraphComponentView::Element() const noexcept {
  return m_element;
}

void ParagraphComponentView::OnPointerEvent(
    winrt::Microsoft::ReactNative::ReactPointerEventArgs const &args) const noexcept {
  // Identify the specific run with hit testing
  const auto point = args.Args().GetCurrentPoint(m_element).Position();
  for (const auto inlineItem : m_element.Inlines()) {
    auto run = inlineItem.try_as<xaml::Documents::Run>();
    if (!run) {
      if (const auto span = inlineItem.try_as<xaml::Documents::Span>()) {
        run = span.Inlines().GetAt(0).as<xaml::Documents::Run>();
      }
    }

    const auto tag = GetTag(run);
    if (m_fragmentEventEmitters.count(static_cast<facebook::react::Tag>(tag)) &&
        TextHitTestUtils::HitTest(run, point)) {
      args.Target(run);
    }
  }

  if (args.Kind() == winrt::Microsoft::ReactNative::PointerEventKind::CaptureLost) {
    if (!m_selectionChanged || !*m_selectionChanged) {
      args.Kind(winrt::Microsoft::ReactNative::PointerEventKind::End);
    }
    *m_selectionChanged = false;
  }

  Super::OnPointerEvent(args);
}

void ParagraphComponentView::ToggleTouchEvents(bool isSelectable) {
  if (isSelectable) {
    m_selectionChangedRevoker = m_element.SelectionChanged(
        winrt::auto_revoke, [selectionChanged = m_selectionChanged](const auto &sender, auto &&) {
          const auto textBlock = sender.as<xaml::Controls::TextBlock>();
          *selectionChanged =
              *selectionChanged || textBlock.SelectionStart().Offset() != textBlock.SelectionEnd().Offset();
        });

    // Get ReactRootView from current element
    xaml::DependencyObject rootView = m_element;
    while (rootView) {
      if (rootView.try_as<winrt::Microsoft::ReactNative::ReactRootView>()) {
        break;
      } else {
        rootView = winrt::VisualTreeHelper::GetParent(rootView);
      }
    }

    auto contextSelf = winrt::get_self<winrt::Microsoft::ReactNative::implementation::ReactContext>(m_context.Handle());
    m_touchEventHandler = std::make_shared<FabricTouchEventHandler>(contextSelf->GetInner());
    m_touchEventHandler->AddTouchHandlers(m_element, rootView, true);
    EnsureUniqueTextFlyoutForXamlIsland(m_element);
  } else {
    m_touchEventHandler = nullptr;
    m_selectionChangedRevoker.revoke();
    *m_selectionChanged = false;
    ClearUniqueTextFlyoutForXamlIsland(m_element);
  }
}

struct KeyPressState {
  std::optional<winrt::Windows::System::VirtualKey> lastKey;
  xaml::UIElement::PreviewKeyUp_revoker keyUpRevoker;
  xaml::UIElement::PointerReleased_revoker pointerReleasedRevoker;
};

xaml::Documents::Hyperlink ParagraphComponentView::CreateHyperlink(facebook::react::Tag tag) {
  xaml::Documents::Hyperlink hyperlink{};

  // Underline should be handled by base class using 'textDecorationLine' prop
  hyperlink.UnderlineStyle(xaml::Documents::UnderlineStyle::None);

  // Pointer click events should be handled by the TouchEventHandler. The only
  // condition where we want to send "onClick" events is when the user invokes
  // the hyperlink while it has focus by pressing "Enter" or "Space".
  const auto keyPressState = std::make_shared<KeyPressState>();
  hyperlink.GotFocus([keyPressState](auto &&sender, auto &&) {
    const auto hyperlink = sender.as<xaml::Documents::Hyperlink>();
    const auto textBlock = hyperlink.ContentStart().VisualParent().try_as<xaml::Controls::TextBlock>();
    if (textBlock) {
      keyPressState->keyUpRevoker = textBlock.PreviewKeyUp(
          winrt::auto_revoke, [keyPressState](auto &&, xaml::Input::KeyRoutedEventArgs const &args) {
            keyPressState->lastKey = args.Key();
          });
      keyPressState->pointerReleasedRevoker = textBlock.PointerReleased(
          winrt::auto_revoke, [keyPressState](auto &&...) { keyPressState->lastKey = std::nullopt; });
    }
  });

  hyperlink.LostFocus([keyPressState](auto &&...) {
    keyPressState->keyUpRevoker.revoke();
    keyPressState->pointerReleasedRevoker.revoke();
    keyPressState->lastKey = std::nullopt;
  });

  hyperlink.Click([this, keyPressState, tag](winrt::IInspectable const &sender, auto &&) {
    if (const auto eventEmitter = GetEventEmitter(tag)) {
      const auto hyperlink = sender.as<xaml::Documents::Hyperlink>();
      auto lastKey = keyPressState->lastKey;

      // When the parent TextBlock is not selectable, `PointerPressed` events are
      // marked `Handled` and thus do not reach the root view gesture handler.
      // The last key state is cleared on `PointerReleased`, so this workaround
      // treats `Click` events from pointers as `Enter` key presses.
      // TODO(T88090620): Add pointer data to event for pointer `Click` events.
      if (!lastKey) {
        const auto textBlock = hyperlink.ContentStart().VisualParent().try_as<xaml::Controls::TextBlock>();
        if (textBlock && !textBlock.IsTextSelectionEnabled()) {
          lastKey = winrt::Windows::System::VirtualKey::Enter;
        }
      }

      if (lastKey == winrt::Windows::System::VirtualKey::Enter ||
          lastKey == winrt::Windows::System::VirtualKey::Space) {
        keyPressState->lastKey = std::nullopt;
        eventEmitter->onClick();
      }
    }
  });

  return hyperlink;
}

} // namespace Microsoft::ReactNative
