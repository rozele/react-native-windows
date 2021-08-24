// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "TextHitTestVisitor.h"
#include <Utils/TextHitTestUtils.h>
#include <Views/RawTextViewManager.h>
#include <Views/VirtualTextViewManager.h>

namespace winrt {
using namespace xaml::Documents;
} // namespace winrt

namespace Microsoft::ReactNative {

void TextHitTestVisitor::VisitChildren(ShadowNodeBase *node) {
  for (auto childTag : node->m_children) {
    Visit(GetShadowNode(childTag));

    // Terminate recursion when we find a hit target
    if (targetView) {
      break;
    }    
  }
}

void TextHitTestVisitor::VisitRawText(ShadowNodeBase *node) {
  if (m_hasPressableAncestor) {
    const auto run = node->GetView().as<winrt::Run>();
    targetView = TextHitTestUtils::HitTest(run, m_point) ? run : nullptr;
  }
}

void TextHitTestVisitor::VisitVirtualText(ShadowNodeBase *node) {
  const auto textNode = static_cast<VirtualTextShadowNode *>(node);

  // Skip nodes without pressable descendants if no pressable ancestors
  if (textNode->hasDescendantPressable || m_hasPressableAncestor) {
    const auto initialPressableCount = pressableCount;
    const auto hadPressableAncestor = m_hasPressableAncestor;
    if (textNode->isPressable) {
      ++pressableCount;
      m_hasPressableAncestor = true;
    }

    Super::VisitVirtualText(node);

    m_hasPressableAncestor = hadPressableAncestor;

    // Set the target view to the first parent of run
    if (targetView && targetView.try_as<winrt::Run>()) {
      targetView = node->GetView();
    }

    // Lazily reset pressable descendants flag
    if (pressableCount == initialPressableCount) {
      textNode->hasDescendantPressable = false;
    }
  }
}

} // namespace Microsoft::ReactNative
