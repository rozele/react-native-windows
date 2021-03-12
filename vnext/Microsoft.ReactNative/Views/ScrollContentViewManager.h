// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Views/FrameworkElementViewManager.h>

namespace react::uwp {

class ScrollContentViewManager : public FrameworkElementViewManager {
  using Super = FrameworkElementViewManager;

 public:
  ScrollContentViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;

  void AddView(const XamlView &parent, const XamlView &child, int64_t index) override;
  void RemoveAllChildren(const XamlView &parent) override;
  void RemoveChildAt(const XamlView &parent, int64_t index) override;

 protected:
  XamlView CreateViewCore(int64_t tag) override;
};

} // namespace react::uwp
