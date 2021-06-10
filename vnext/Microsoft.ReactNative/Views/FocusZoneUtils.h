// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <Views/ViewManagerBase.h>

namespace react::uwp {

void SetIsFocusZone(xaml::UIElement element, ViewManagerBase *viewManager, bool isFocusZone);
void UpdateFocusZoneXYFocusNavigationStrategy(xaml::UIElement focusZoneElement, const folly::dynamic &propertyValue);

} // namespace react::uwp
