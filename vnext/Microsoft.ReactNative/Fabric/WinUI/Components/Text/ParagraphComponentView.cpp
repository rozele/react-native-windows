// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ParagraphComponentView.h"

#include <UI.Input.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <UI.Xaml.Input.h>
#include <Utils/ValueUtils.h>
#include <Views/Text/TextHitTestUtils.h>
#include <dwrite.h>
#include <react/renderer/components/text/ParagraphShadowNode.h>
#include <react/renderer/components/text/ParagraphState.h>
#include <unicode.h>
#include "XamlView.h"

namespace Microsoft::ReactNative {

ParagraphComponentView::ParagraphComponentView() {
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

void ParagraphComponentView::updateState(
    facebook::react::State::Shared const &state,
    facebook::react::State::Shared const &oldState) noexcept {
  const auto &newState = *std::static_pointer_cast<facebook::react::ParagraphShadowNode::ConcreteState const>(state);

  m_element.Inlines().Clear();

  for (const auto &fragment : newState.getData().attributedString.getFragments()) {
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

    run.Text(winrt::to_hstring(fragment.string));
    run.FontFamily(xaml::Media::FontFamily(
        fragment.textAttributes.fontFamily.empty() ? L"Segoe UI"
                                                   : winrt::to_hstring(fragment.textAttributes.fontFamily)));

    run.FontWeight(
        winrt::Windows::UI::Text::FontWeight{static_cast<uint16_t>(fragment.textAttributes.fontWeight.value_or(
            static_cast<facebook::react::FontWeight>(DWRITE_FONT_WEIGHT_REGULAR)))});

    run.FontSize(fragment.textAttributes.fontSize);
    run.Foreground(fragment.textAttributes.foregroundColor.AsWindowsBrush());
    inlines.Append(run);
  }
}
void ParagraphComponentView::updateLayoutMetrics(
    facebook::react::LayoutMetrics const &layoutMetrics,
    facebook::react::LayoutMetrics const &oldLayoutMetrics) noexcept {
  // Set Position & Size Properties

  m_layoutMetrics = layoutMetrics;

  winrt::Microsoft::ReactNative::ViewPanel::SetLeft(m_element, layoutMetrics.frame.origin.x);
  winrt::Microsoft::ReactNative::ViewPanel::SetTop(m_element, layoutMetrics.frame.origin.y);

  m_element.Width(layoutMetrics.frame.size.width);
  m_element.Height(layoutMetrics.frame.size.height);
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

  Super::OnPointerEvent(args);
}

} // namespace Microsoft::ReactNative
