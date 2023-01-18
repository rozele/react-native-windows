// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ParagraphComponentView.h"

#include <IReactContext.h>
#include <UI.Input.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <UI.Xaml.Media.h>
#include <Utils/TransformableText.h>
#include <Utils/ValueUtils.h>
#include <Views/Text/TextHitTestUtils.h>
#include <dwrite.h>
#include <react/renderer/components/text/ParagraphShadowNode.h>
#include <react/renderer/components/text/ParagraphState.h>
#include <unicode.h>
#include "XamlView.h"

namespace Microsoft::ReactNative {

ParagraphComponentView::ParagraphComponentView(winrt::Microsoft::ReactNative::ReactContext const &reactContext)
    : m_context(reactContext) {
  static auto const defaultProps = std::make_shared<facebook::react::ParagraphProps const>();
  m_props = defaultProps;

  m_element.TextWrapping(xaml::TextWrapping::Wrap); // Default behavior in React Native
}

std::vector<facebook::react::ComponentDescriptorProvider>
ParagraphComponentView::supplementalComponentDescriptorProviders() noexcept {
  return {};
}

void ParagraphComponentView::updateProps(
    facebook::react::Props::Shared const &props,
    facebook::react::Props::Shared const &oldProps) noexcept {
  const auto &oldViewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(m_props);
  const auto &newViewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(props);

  if (oldViewProps.textAttributes.foregroundColor != newViewProps.textAttributes.foregroundColor) {
    if (newViewProps.textAttributes.foregroundColor)
      m_element.Foreground(newViewProps.textAttributes.foregroundColor.AsWindowsBrush());
    else
      m_element.ClearValue(::xaml::Controls::TextBlock::ForegroundProperty());
  }

  if (oldViewProps.textAttributes.fontSize != newViewProps.textAttributes.fontSize) {
    if (std::isnan(newViewProps.textAttributes.fontSize))
      m_element.ClearValue(::xaml::Controls::TextBlock::FontSizeProperty());
    else
      m_element.FontSize(newViewProps.textAttributes.fontSize);
  }

  if (oldViewProps.textAttributes.fontWeight != newViewProps.textAttributes.fontWeight) {
    m_element.FontWeight(
        winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(newViewProps.textAttributes.fontWeight.value_or(
            static_cast<facebook::react::FontWeight>(DWRITE_FONT_WEIGHT_REGULAR)))});
  }

  if (oldViewProps.textAttributes.fontStyle != newViewProps.textAttributes.fontStyle) {
    switch (newViewProps.textAttributes.fontStyle.value_or(facebook::react::FontStyle::Normal)) {
      case facebook::react::FontStyle::Italic:
        m_element.FontStyle(winrt::Windows::UI::Text::FontStyle::Italic);
        break;
      case facebook::react::FontStyle::Normal:
        m_element.FontStyle(winrt::Windows::UI::Text::FontStyle::Normal);
        break;
      case facebook::react::FontStyle::Oblique:
        m_element.FontStyle(winrt::Windows::UI::Text::FontStyle::Oblique);
        break;
      default:
        assert(false);
    }
  }

  if (oldViewProps.textAttributes.fontFamily != newViewProps.textAttributes.fontFamily) {
    if (newViewProps.textAttributes.fontFamily.empty())
      m_element.FontFamily(xaml::Media::FontFamily(L"Segoe UI"));
    else
      m_element.FontFamily(
          xaml::Media::FontFamily(Microsoft::Common::Unicode::Utf8ToUtf16(newViewProps.textAttributes.fontFamily)));
  }

  if (oldViewProps.isSelectable != newViewProps.isSelectable) {
    m_element.IsTextSelectionEnabled(newViewProps.isSelectable);
    ToggleTouchEvents(newViewProps.isSelectable);
  }

  if (oldViewProps.backgroundColor != newViewProps.backgroundColor) {
    if (m_inheritsBackground.size() > 0) {
      const auto newBrush = newViewProps.backgroundColor ? newViewProps.backgroundColor.AsWindowsBrush()
                                                         : xaml::Media::SolidColorBrush{winrt::Colors::Transparent()};
      assert(m_inheritsBackground.size() == m_element.TextHighlighters().Size());
      for (auto i = 0; i < m_inheritsBackground.size(); ++i) {
        if (m_inheritsBackground[i]) {
          m_element.TextHighlighters().GetAt(i).Background(newBrush);
        }
      }
    }
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

  Super::updateProps(props, oldProps);
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
  const auto &newState = *std::static_pointer_cast<facebook::react::ParagraphShadowNode::ConcreteState const>(state);
  const auto &attributedString = newState.getData().attributedString;
  if (oldState) {
    const auto &oldTextState =
        *std::static_pointer_cast<facebook::react::ParagraphShadowNode::ConcreteState const>(oldState);
    const auto &oldAttributedString = oldTextState.getData().attributedString;
    if (attributedString.isContentEqual(oldAttributedString)) {
      return;
    }
  }

  m_element.Inlines().Clear();
  m_element.TextHighlighters().Clear();
  m_inheritsBackground.clear();
  m_fragmentEventEmitters.clear();

  // TODO(T142895298): Apply ViewProps::backgroundColor prop when no state change occurs
  int32_t position = 0;
  const auto &viewProps = *std::static_pointer_cast<const facebook::react::ParagraphProps>(m_props);
  const auto defaultBackground = facebook::react::isColorMeaningful(viewProps.backgroundColor)
      ? viewProps.backgroundColor.AsWindowsBrush()
      : nullptr;

  for (const auto &fragment : attributedString.getFragments()) {
    auto inlines = m_element.Inlines();

    if (auto tdlt = fragment.textAttributes.textDecorationLineType; tdlt &&
        (*tdlt == facebook::react::TextDecorationLineType::Underline ||
         *tdlt == facebook::react::TextDecorationLineType::UnderlineStrikethrough)) {
      auto underline = xaml::Documents::Underline();
      inlines.Append(underline);
      inlines = underline.Inlines();
    }

    if (fragment.textAttributes.fontStyle == facebook::react::FontStyle::Italic ||
        fragment.textAttributes.fontStyle == facebook::react::FontStyle::Oblique) {
      auto italic = xaml::Documents::Italic();
      inlines.Append(italic);
      inlines = italic.Inlines();
    }

    const auto run = xaml::Documents::Run();
    const auto tag = fragment.parentShadowView.tag;
    SetTag(run, tag);
    // TODO(T140425180): use the `pressable` prop to determine whether we need to hit test.
    if (fragment.parentShadowView.eventEmitter) {
      m_fragmentEventEmitters[tag] =
          std::static_pointer_cast<facebook::react::ViewEventEmitter const>(fragment.parentShadowView.eventEmitter);
    }

    run.Text(TransformableText::TransformText(
        winrt::to_hstring(fragment.string), ConvertTextTransform(fragment.textAttributes.textTransform)));
    run.FontFamily(xaml::Media::FontFamily(
        fragment.textAttributes.fontFamily.empty() ? L"Segoe UI"
                                                   : winrt::to_hstring(fragment.textAttributes.fontFamily)));

    run.FontWeight(
        winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(fragment.textAttributes.fontWeight.value_or(
            static_cast<facebook::react::FontWeight>(DWRITE_FONT_WEIGHT_REGULAR)))});

    run.FontSize(fragment.textAttributes.fontSize);

    const auto foreground = fragment.textAttributes.foregroundColor.AsWindowsBrush();
    run.Foreground(foreground);

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
    m_inheritsBackground.push_back(!hasNestedBackground);

    position += length;
  }
}

void ParagraphComponentView::finalizeUpdates(RNComponentViewUpdateMask updateMask) noexcept {}
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
  } else {
    m_touchEventHandler = nullptr;
    m_selectionChangedRevoker.revoke();
    *m_selectionChanged = false;
  }
}

} // namespace Microsoft::ReactNative
