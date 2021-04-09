// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "RawTextViewManager.h"
#include "TextViewManager.h"
#include "VirtualTextViewManager.h"

#include <Views/ShadowNodeBase.h>

#include <INativeUIManager.h>
#include <Utils/ValueUtils.h>

#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>
#include <winrt/Windows.Foundation.h>

namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace xaml;
using namespace xaml::Controls;
using namespace xaml::Documents;
using namespace xaml::Media;
} // namespace winrt

namespace react::uwp {

RawTextViewManager::RawTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

const char *RawTextViewManager::GetName() const {
  return "RCTRawText";
}

XamlView RawTextViewManager::CreateViewCore(int64_t /*tag*/) {
  winrt::Run run;
  return run;
}

bool RawTextViewManager::UpdateProperty(
    ShadowNodeBase *nodeToUpdate,
    const std::string &propertyName,
    const folly::dynamic &propertyValue) {
  auto run = nodeToUpdate->GetView().as<winrt::Run>();
  if (run == nullptr)
    return true;

  if (propertyName == "text") {
    run.Text(react::uwp::asHstring(propertyValue));
    static_cast<RawTextShadowNode *>(nodeToUpdate)->originalText = winrt::hstring{};
    NotifyAncestorsTextChanged(nodeToUpdate);
  } else {
    return Super::UpdateProperty(nodeToUpdate, propertyName, propertyValue);
  }
  return true;
}

void RawTextViewManager::NotifyAncestorsTextChanged(ShadowNodeBase *nodeToUpdate) {
  if (auto instance = this->m_wkReactInstance.lock()) {
    auto host = instance->NativeUIManager()->getHost();
    ShadowNodeBase *parent = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(nodeToUpdate->GetParent()));
    TextTransform textTransform = TextTransform::Undefined;
    while (parent) {
      auto viewManager = parent->GetViewManager();
      const auto nodeType = viewManager->GetName();
      if (!std::strcmp(nodeType, "RCTText")) {
        const auto textViewManager = static_cast<TextViewManager *>(viewManager);
        if (textTransform == TextTransform::Undefined) {
          textTransform = textViewManager->GetTextTransformValue(parent);
        }

        VirtualTextShadowNode::ApplyTextTransform(
            *nodeToUpdate, textTransform, /* forceUpdate = */ false, /* isRoot = */ false);

        if (parent->m_children.size() == 1) {
          auto view = parent->GetView();
          auto textBlock = view.try_as<winrt::TextBlock>();
          if (textBlock != nullptr) {
            const auto run = nodeToUpdate->GetView().try_as<winrt::Run>();
            if (run != nullptr) {
              textBlock.Text(run.Text());
            }
          }
        }

        (static_cast<TextViewManager *>(viewManager))->OnDescendantTextPropertyChanged(parent);
      } else if (!std::strcmp(nodeType, "RCTVirtualText") && textTransform == TextTransform::Undefined) {
        textTransform = static_cast<VirtualTextShadowNode *>(parent)->textTransform;
      }
      parent = static_cast<ShadowNodeBase *>(host->FindShadowNodeForTag(parent->GetParent()));
    }
  }
}

void RawTextViewManager::SetLayoutProps(
    ShadowNodeBase & /*nodeToUpdate*/,
    const XamlView & /*viewToUpdate*/,
    float /*left*/,
    float /*top*/,
    float /*width*/,
    float /*height*/) {}

bool RawTextViewManager::RequiresYogaNode() const {
  return false;
}

} // namespace react::uwp
