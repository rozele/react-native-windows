// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <Views/ViewViewManager.h>
#include <Views/ViewPanel.h>

namespace react::uwp {

class FocusZoneViewManager : public ViewViewManager {
  using Super = ViewViewManager;

 public:
  FocusZoneViewManager(const std::shared_ptr<IReactInstance> &reactInstance);

  const char *GetName() const override;

  folly::dynamic GetNativeProps() const override;
  facebook::react::ShadowNode *createShadow() const override;
};

} // namespace react::uwp
