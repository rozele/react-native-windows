// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Views/FrameworkElementViewManager.h>

namespace Microsoft::ReactNative {

class ScrollContentViewManager : public FrameworkElementViewManager {
  using Super = FrameworkElementViewManager;

 public:
  ScrollContentViewManager(const Mso::React::IReactContext &context);

  const wchar_t *GetName() const override;

  ShadowNode *createShadow() const override;

 protected:
  XamlView CreateViewCore(int64_t tag, const winrt::Microsoft::ReactNative::JSValueObject &) override;
};

} // namespace Microsoft::ReactNative
