// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Views/VirtualTextViewManager.h>

namespace react::uwp {

class HyperlinkTextViewManager : public VirtualTextViewManager {
  using Super = VirtualTextViewManager;

 public:
  HyperlinkTextViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;
  folly::dynamic GetExportedCustomDirectEventTypeConstants() const override;

 protected:
  XamlView CreateViewCore(int64_t tag) override;
};

} // namespace react::uwp
