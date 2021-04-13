// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "TextViewManager.h"

#include <Views/RawTextViewManager.h>
#include <Views/ShadowNodeBase.h>
#include <Views/VirtualTextViewManager.h>

#include <UI.Xaml.Automation.Peers.h>
#include <UI.Xaml.Automation.h>
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <Utils/PropertyUtils.h>
#include <Utils/TextHitTestUtils.h>
#include <Utils/TransformableText.h>
#include <Utils/ValueUtils.h>

namespace winrt {
using namespace xaml::Documents;
using namespace xaml::Automation;
using namespace xaml::Automation::Peers;
} // namespace winrt

namespace react::uwp {

class TextShadowNode final : public ShadowNodeBase {
  using Super = ShadowNodeBase;
  friend TextViewManager;

 private:
  ShadowNode *m_firstChildNode;

  std::optional<winrt::Windows::UI::Color> m_ColorValue = std::nullopt;
  int32_t m_prevCursorEnd = 0;

 public:
  TextShadowNode() {
    m_firstChildNode = nullptr;
  };
  bool ImplementsPadding() override {
    return true;
  }

  void AddView(ShadowNode &child, int64_t index) override {
    auto &childNode = static_cast<ShadowNodeBase &>(child);
    VirtualTextShadowNode::ApplyTextTransform(
        childNode, textTransform, /* forceUpdate = */ false, /* isRoot = */ false);

    if (index == 0) {
      auto run = childNode.GetView().try_as<winrt::Run>();
      if (run != nullptr) {
        m_firstChildNode = &child;
        auto textBlock = this->GetView().as<xaml::Controls::TextBlock>();
        textBlock.Text(run.Text());
        if (m_ColorValue) {
          AddHighlighter(m_ColorValue.value(), textBlock.Text().size());
        }

        m_prevCursorEnd += textBlock.Text().size();

        return;
      }
    } else if (index == 1 && m_firstChildNode != nullptr) {
      auto textBlock = this->GetView().as<xaml::Controls::TextBlock>();
      textBlock.ClearValue(xaml::Controls::TextBlock::TextProperty());
      Super::AddView(*m_firstChildNode, 0);
      m_firstChildNode = nullptr;
    }

    Super::AddView(child, index);

    if (auto run = static_cast<ShadowNodeBase&>(child).GetView().try_as<winrt::Run>()) {
      if (m_ColorValue) {
        AddHighlighter(m_ColorValue.value(), run.Text().size());
      }
      m_prevCursorEnd += run.Text().size();
    } else if (auto span = static_cast<ShadowNodeBase &>(child).GetView().try_as<winrt::Span>()) {
      const auto &virtualTextNode = static_cast<VirtualTextShadowNode &>(child);
      AddNestedTextHighlighter(m_ColorValue, span, virtualTextNode.m_highlightData);
      pressableCount += virtualTextNode.m_pressableCount;
    }
  }

  void AddNestedTextHighlighter(
      const std::optional<winrt::Windows::UI::Color>& parentColor,
      winrt::Span& span,
      VirtualTextShadowNode::HighlightData highData) {
    if (!highData.color && parentColor) {
      highData.color = parentColor;
    }

    for (const auto& el : span.Inlines()) {
      if (auto run = el.try_as<winrt::Run>()) {
        if (highData.color) {
          AddHighlighter(highData.color.value(), run.Text().size());
        }
        m_prevCursorEnd += run.Text().size();
      } else if (auto spanChild = el.try_as<winrt::Span>()) {
        AddNestedTextHighlighter(highData.color, spanChild, highData.data[highData.spanIdx++]);
      }
    }
  }

  void AddHighlighter(const winrt::Windows::UI::Color& background, size_t runSize) {
    auto newHigh = winrt::TextHighlighter{};
    newHigh.Background(react::uwp::SolidBrushFromColor(background));

    winrt::TextRange newRange{m_prevCursorEnd, static_cast<int32_t>(runSize)};
    newHigh.Ranges().Append(newRange);

    this->GetView().as<xaml::Controls::TextBlock>().TextHighlighters().Append(newHigh);
  }

  void removeAllChildren() override {
    m_firstChildNode = nullptr;
    Super::removeAllChildren();
  }

  void RemoveChildAt(int64_t indexToRemove) override {
    if (indexToRemove == 0) {
      m_firstChildNode = nullptr;
    }
    Super::RemoveChildAt(indexToRemove);
  }

  int64_t GetReactTagAtPoint(const winrt::Point &point) {
    if (pressableCount > 0) {
      const auto textPointer = useBlockHitTest
          ? TextHitTestUtils::GetPositionFromPoint(GetView().as<xaml::Controls::TextBlock>(), point)
          : VirtualTextShadowNode::HitTest(*this, point, /* hasPressableParent = */ false);

      if (textPointer != nullptr) {
        auto inlineTag = GetTag(textPointer.Parent());
        if (inlineTag != -1) {
          if (auto instance = GetViewManager()->GetReactInstance().lock()) {
            auto host = instance->NativeUIManager()->getHost();
            const auto node = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(inlineTag));
            // React Native does not support events targeted to raw text nodes.
            // Get the parent tag instead.
            if (!std::strcmp(node->GetViewManager()->GetName(), "RCTRawText")) {
              inlineTag = node->GetParent();
            }
            return inlineTag;
          }
        }
      }
    }

    return m_tag;
  }

  TextTransform textTransform{TextTransform::Undefined};
  int pressableCount{0};
  bool useBlockHitTest{false};
};

TextViewManager::TextViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

facebook::react::ShadowNode *TextViewManager::createShadow() const {
  return new TextShadowNode();
}

const char *TextViewManager::GetName() const {
  return "RCTText";
}

XamlView TextViewManager::CreateViewCore(int64_t /*tag*/) {
  auto textBlock = xaml::Controls::TextBlock();
  textBlock.TextWrapping(xaml::TextWrapping::Wrap); // Default behavior in React Native
  return textBlock;
}

bool TextViewManager::UpdateProperty(
    ShadowNodeBase *nodeToUpdate,
    const std::string &propertyName,
    const folly::dynamic &propertyValue) {
  auto textBlock = nodeToUpdate->GetView().as<xaml::Controls::TextBlock>();
  if (textBlock == nullptr)
    return true;

  if (TryUpdateForeground(textBlock, propertyName, propertyValue)) {
  } else if (TryUpdateFontProperties(textBlock, propertyName, propertyValue)) {
  } else if (propertyName == "textTransform") {
    auto textNode = static_cast<TextShadowNode *>(nodeToUpdate);
    textNode->textTransform = TransformableText::GetTextTransform(propertyValue);
    VirtualTextShadowNode::ApplyTextTransform(
        *textNode, textNode->textTransform, /* forceUpdate = */ true, /* isRoot = */ true);
  } else if (TryUpdatePadding(nodeToUpdate, textBlock, propertyName, propertyValue)) {
  } else if (TryUpdateTextAlignment(textBlock, propertyName, propertyValue)) {
  } else if (TryUpdateTextTrimming(textBlock, propertyName, propertyValue)) {
  } else if (TryUpdateTextDecorationLine(textBlock, propertyName, propertyValue)) {
  } else if (TryUpdateCharacterSpacing(textBlock, propertyName, propertyValue)) {
  } else if (propertyName == "numberOfLines") {
    if (propertyValue.isNumber()) {
      auto numberLines = static_cast<int32_t>(propertyValue.asDouble());
      if (numberLines == 1) {
        textBlock.TextWrapping(xaml::TextWrapping::NoWrap); // setting no wrap for single line
                                         // text for better trimming
                                         // experience
      } else {
        textBlock.TextWrapping(xaml::TextWrapping::Wrap);
      }
      textBlock.MaxLines(numberLines);
    } else if (propertyValue.isNull()) {
      textBlock.TextWrapping(xaml::TextWrapping::Wrap); // set wrapping back to default
      textBlock.ClearValue(xaml::Controls::TextBlock::MaxLinesProperty());
    }
  } else if (propertyName == "lineHeight") {
    if (propertyValue.isNumber())
      textBlock.LineHeight(static_cast<int32_t>(propertyValue.asDouble()));
    else if (propertyValue.isNull())
      textBlock.ClearValue(xaml::Controls::TextBlock::LineHeightProperty());
  } else if (propertyName == "selectable") {
    if (propertyValue.isBool())
      textBlock.IsTextSelectionEnabled(propertyValue.asBool());
    else if (propertyValue.isNull())
      textBlock.ClearValue(xaml::Controls::TextBlock::IsTextSelectionEnabledProperty());
  } else if (propertyName == "allowFontScaling") {
    if (propertyValue.isBool()) {
      textBlock.IsTextScaleFactorEnabled(propertyValue.asBool());
    } else {
      textBlock.ClearValue(xaml::Controls::TextBlock::IsTextScaleFactorEnabledProperty());
    }
  } else if (propertyName == "selectionColor") {
    if (IsValidColorValue(propertyValue)) {
      textBlock.SelectionHighlightColor(SolidColorBrushFrom(propertyValue));
    } else
      textBlock.ClearValue(
          xaml::Controls::TextBlock::SelectionHighlightColorProperty());
  } else if (propertyName == "backgroundColor") {
    if (react::uwp::IsValidColorValue(propertyValue)) {
      static_cast<TextShadowNode*>(nodeToUpdate)->m_ColorValue = react::uwp::ColorFrom(propertyValue);
    }
  } else if (propertyName == "hitTestStrategy") {
    if (propertyValue.isString()) {
      static_cast<TextShadowNode *>(nodeToUpdate)->useBlockHitTest = propertyValue.asString() == "block";
    } else if (propertyValue.isNull()) {
      static_cast<TextShadowNode *>(nodeToUpdate)->useBlockHitTest = false;
    }
  } else {
    return Super::UpdateProperty(nodeToUpdate, propertyName, propertyValue);
  }
  return true;
}

void TextViewManager::AddView(const XamlView &parent, const XamlView &child, int64_t index) {
  auto textBlock(parent.as<xaml::Controls::TextBlock>());
  auto childInline(child.as<winrt::Inline>());
  textBlock.Inlines().InsertAt(static_cast<uint32_t>(index), childInline);
}

void TextViewManager::RemoveAllChildren(const XamlView &parent) {
  auto textBlock(parent.as<xaml::Controls::TextBlock>());
  textBlock.Inlines().Clear();
}

void TextViewManager::RemoveChildAt(const XamlView &parent, int64_t index) {
  auto textBlock(parent.as<xaml::Controls::TextBlock>());
  return textBlock.Inlines().RemoveAt(static_cast<uint32_t>(index));
}

YGMeasureFunc TextViewManager::GetYogaCustomMeasureFunc() const {
  return DefaultYogaSelfMeasureFunc;
}

void TextViewManager::OnDescendantTextPropertyChanged(ShadowNodeBase *node) {
  if (auto element = node->GetView().try_as<xaml::Controls::TextBlock>()) {
    // If name is set, it's controlled by accessibilityLabel, and it's already
    // handled in FrameworkElementViewManager. Here it only handles when name is
    // not set.
    if (xaml::Automation::AutomationProperties::GetLiveSetting(element) != winrt::AutomationLiveSetting::Off &&
        xaml::Automation::AutomationProperties::GetName(element).empty() &&
        xaml::Automation::AutomationProperties::GetAccessibilityView(element) != winrt::Peers::AccessibilityView::Raw) {
      if (auto peer = xaml::Automation::Peers::FrameworkElementAutomationPeer::FromElement(element)) {
        peer.RaiseAutomationEvent(winrt::AutomationEvents::LiveRegionChanged);
      }
    }
  }
}

TextTransform TextViewManager::GetTextTransformValue(ShadowNodeBase *node) {
  if (!std::strcmp(node->GetViewManager()->GetName(), GetName())) {
    return static_cast<TextShadowNode *>(node)->textTransform;
  }

  return TextTransform::Undefined;
}

void TextViewManager::AddToPressableCount(ShadowNodeBase *node, int pressableCount) {
  if (!std::strcmp(node->GetViewManager()->GetName(), GetName())) {
    const auto textNode = static_cast<TextShadowNode *>(node);
    textNode->pressableCount += pressableCount;
  }
}

int64_t TextViewManager::GetReactTagAtPoint(ShadowNodeBase *node, const winrt::Point &point) {
  if (!std::strcmp(node->GetViewManager()->GetName(), "RCTText")) {
    const auto textNode = static_cast<TextShadowNode *>(node);
    return textNode->GetReactTagAtPoint(point);
  }

  return node->m_tag;
}

} // namespace react::uwp
