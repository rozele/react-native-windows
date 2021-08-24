// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "TextVisitor.h"

namespace Microsoft::ReactNative {

class TextHitTestVisitor : public TextVisitor {
  using Super = TextVisitor;

 public:
  int64_t pressableCount{0};
  xaml::DependencyObject targetView{nullptr};

  TextHitTestVisitor(winrt::Point const &point) : m_point{point} {}

protected:
  void VisitChildren(ShadowNodeBase *node) override;

  void VisitRawText(ShadowNodeBase *node) override;

  void VisitVirtualText(ShadowNodeBase *node) override;

 private:
  winrt::Point m_point;
  bool m_hasPressableAncestor{false};
};

} // namespace Microsoft::ReactNative
