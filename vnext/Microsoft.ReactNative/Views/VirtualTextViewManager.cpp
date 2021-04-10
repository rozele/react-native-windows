// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "VirtualTextViewManager.h"

#include <UI.Xaml.Documents.h>
#include <Utils/PropertyUtils.h>
#include <Utils/ShadowNodeTypeUtils.h>
#include <Utils/TextHitTestUtils.h>
#include <Utils/TransformableText.h>
#include <Utils/ValueUtils.h>
#include <Views/Text/TextVisitors.h>

namespace winrt {
using namespace Windows::UI;
using namespace xaml;
using namespace xaml::Controls;
using namespace xaml::Documents;
} // namespace winrt

namespace Microsoft::ReactNative {

void VirtualTextShadowNode::AddView(ShadowNode &child, int64_t index) {
  auto &childNode = static_cast<ShadowNodeBase &>(child);
  ApplyTextTransformToChild(&child);
  auto propertyChangeType = PropertyChangeType::Text;
  if (IsVirtualTextShadowNode(&childNode)) {
    const auto &childTextNode = static_cast<VirtualTextShadowNode &>(childNode);
    AddToPressableCount(childTextNode.m_pressableCount);
    propertyChangeType |=
        childTextNode.hasDescendantTextHighlighter ? PropertyChangeType::AddHighlight : PropertyChangeType::None;
  }
  Super::AddView(child, index);
  NotifyAncestorsTextPropertyChanged(this, propertyChangeType);
}

void VirtualTextShadowNode::RemoveChildAt(int64_t indexToRemove) {
  Super::RemoveChildAt(indexToRemove);
  NotifyAncestorsTextPropertyChanged(this, PropertyChangeType::Text);
}

void VirtualTextShadowNode::removeAllChildren() {
  Super::removeAllChildren();
  NotifyAncestorsTextPropertyChanged(this, PropertyChangeType::Text);
}

void VirtualTextShadowNode::onDropViewInstance() {
  AddToPressableCount(-m_pressableCount);
  Super::onDropViewInstance();
}

void VirtualTextShadowNode::AddToPressableCount(int count) {
  m_pressableCount += count;
  if (const auto uiManager = GetNativeUIManager(GetViewManager()->GetReactContext()).lock()) {
    if (m_parent != -1) {
      const auto parentNode = static_cast<ShadowNodeBase *>(uiManager->getHost()->FindShadowNodeForTag(m_parent));
      const auto viewManager = parentNode->GetViewManager();
      if (!std::wcscmp(viewManager->GetName(), L"RCTText")) {
        static_cast<TextViewManager *>(viewManager)->AddToPressableCount(parentNode, count);
      } else if (!std::wcscmp(parentNode->GetViewManager()->GetName(), L"RCTVirtualText")) {
        static_cast<VirtualTextShadowNode *>(parentNode)->AddToPressableCount(count);
      }
    }
  }
}

void VirtualTextShadowNode::SetPressable(bool isPressable) {
  const auto wasPressable = m_isPressable;
  m_isPressable = isPressable;
  if (!wasPressable && isPressable) {
    AddToPressableCount(1);
  } else if (wasPressable && !isPressable) {
    AddToPressableCount(-1);
  }
}

xaml::Documents::TextPointer VirtualTextShadowNode::HitTest(const ShadowNodeBase &node, const winrt::Point &point) {
  const auto viewManager = node.GetViewManager();
  const auto nodeType = viewManager->GetName();
  if (!std::wcscmp(nodeType, L"RCTRawText")) {
    // Check if the point is within the bounds of the Run
    const auto run = node.GetView().as<winrt::Run>();
    return TextHitTestUtils::GetPositionFromPoint(run, point);
  } else {
    // If the node is a nested Text component, skip if it has no pressable descendants.
    const auto isVirtualText = !std::wcscmp(nodeType, L"RCTVirtualText");
    if (!isVirtualText || static_cast<const VirtualTextShadowNode &>(node).m_pressableCount > 0) {
      if (auto uiManager = GetNativeUIManager(viewManager->GetReactContext()).lock()) {
        // Otherwise, visit each child
        for (const auto childTag : node.m_children) {
          const auto childNode = static_cast<ShadowNodeBase *>(uiManager->getHost()->FindShadowNodeForTag(childTag));
          const auto textPointer = HitTest(*childNode, point);
          if (textPointer != nullptr) {
            return textPointer;
          }
        }
      }
    }
  }

  return nullptr;
}

VirtualTextViewManager::VirtualTextViewManager(const Mso::React::IReactContext &context) : Super(context) {}

const wchar_t *VirtualTextViewManager::GetName() const {
  return L"RCTVirtualText";
}

XamlView VirtualTextViewManager::CreateViewCore(int64_t /*tag*/, const winrt::Microsoft::ReactNative::JSValueObject &) {
  return winrt::Span();
}

bool VirtualTextViewManager::UpdateProperty(
    ShadowNodeBase *nodeToUpdate,
    const std::string &propertyName,
    const winrt::Microsoft::ReactNative::JSValue &propertyValue) {
  auto span = nodeToUpdate->GetView().as<winrt::Span>();
  if (span == nullptr)
    return true;

  // FUTURE: In the future cppwinrt will generate code where static methods on
  // base types can be called.  For now we specify the base type explicitly
  if (TryUpdateForeground<winrt::TextElement>(span, propertyName, propertyValue)) {
    auto node = static_cast<VirtualTextShadowNode *>(nodeToUpdate);
    if (IsValidOptionalColorValue(propertyValue)) {
      node->foregroundColor = OptionalColorFrom(propertyValue);
      const auto propertyChangeType =
          node->foregroundColor ? PropertyChangeType::AddHighlight : PropertyChangeType::RemoveHighlight;
      NotifyAncestorsTextPropertyChanged(node, propertyChangeType);
    }
  } else if (TryUpdateFontProperties<winrt::TextElement>(span, propertyName, propertyValue)) {
  } else if (TryUpdateCharacterSpacing<winrt::TextElement>(span, propertyName, propertyValue)) {
  } else if (TryUpdateTextDecorationLine<winrt::TextElement>(span, propertyName, propertyValue)) {
  } else if (propertyName == "textTransform") {
    auto node = static_cast<VirtualTextShadowNode *>(nodeToUpdate);
    node->textTransform = TransformableText::GetTextTransform(propertyValue);
    UpdateTextTransformForChildren(nodeToUpdate);
  } else if (propertyName == "backgroundColor") {
    auto node = static_cast<VirtualTextShadowNode *>(nodeToUpdate);
    if (IsValidOptionalColorValue(propertyValue)) {
      node->backgroundColor = OptionalColorFrom(propertyValue);
      const auto propertyChangeType =
          node->backgroundColor ? PropertyChangeType::AddHighlight : PropertyChangeType::RemoveHighlight;
      NotifyAncestorsTextPropertyChanged(node, propertyChangeType);
    }
  } else if (propertyName == "isPressable") {
    static_cast<VirtualTextShadowNode *>(nodeToUpdate)->SetPressable(propertyValue.AsBoolean());
  } else {
    return Super::UpdateProperty(nodeToUpdate, propertyName, propertyValue);
  }

  return true;
}

void VirtualTextViewManager::AddView(const XamlView &parent, const XamlView &child, int64_t index) {
  auto span(parent.as<winrt::Span>());
  auto childInline(child.as<winrt::Inline>());
  span.Inlines().InsertAt(static_cast<uint32_t>(index), childInline);
}

void VirtualTextViewManager::RemoveAllChildren(const XamlView &parent) {
  auto span(parent.as<winrt::Span>());
  span.Inlines().Clear();
}

void VirtualTextViewManager::RemoveChildAt(const XamlView &parent, int64_t index) {
  auto span(parent.as<winrt::Span>());
  return span.Inlines().RemoveAt(static_cast<uint32_t>(index));
}

bool VirtualTextViewManager::RequiresYogaNode() const {
  return false;
}

} // namespace Microsoft::ReactNative
