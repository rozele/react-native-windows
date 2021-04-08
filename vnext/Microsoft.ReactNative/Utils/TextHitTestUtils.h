// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Documents.h>

namespace react::uwp {
struct TextHitTestUtils final {
  static xaml::Documents::TextPointer GetPositionFromPoint(
      const xaml::Controls::TextBlock &textBlock,
      const winrt::Point &targetPoint);
  static xaml::Documents::TextPointer GetPositionFromPoint(
      const xaml::Documents::Run &run,
      const winrt::Point &targetPoint);
};
} // namespace react::uwp
