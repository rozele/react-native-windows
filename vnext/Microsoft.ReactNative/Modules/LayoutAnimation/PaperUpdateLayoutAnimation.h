// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <JSValue.h>
#include "PaperLayoutAnimation.h"

namespace Microsoft::ReactNative {

// TODO: support drop view animations
class PaperUpdateLayoutAnimation : public PaperLayoutAnimation {
  using Super = PaperLayoutAnimation;
 public:
#ifndef USE_WINUI3
  xaml::Media::Animation::Storyboard
  CreateAnimation(ShadowNodeBase &node, float left, float top, float width, float height, std::function<void()> &&onComplete) override;

 private:
  xaml::Media::Animation::DoubleAnimation
  CreateTimeline(XamlView const &view, winrt::hstring path, double from, double to);
#endif
};

}; // namespace Microsoft::ReactNative
