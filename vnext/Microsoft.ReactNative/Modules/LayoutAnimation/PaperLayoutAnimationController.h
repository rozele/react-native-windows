// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <JSValue.h>
#include <ShadowNodeBase.h>
#include "PaperUpdateLayoutAnimation.h"
#ifndef USE_WINUI3
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#endif

namespace Microsoft::ReactNative {

struct AnimatedLayoutOperation {
  xaml::Media::Animation::Storyboard storyboard{nullptr};
  float left{0.0f};
  float top{0.0f};
  float width{0.0f};
  float height{0.0f};
};

// TODO: support mount/unmount view animations
// TODO: support WinUI 3
class PaperLayoutAnimationController : public std::enable_shared_from_this<PaperLayoutAnimationController> {
 public:
  void ConfigureLayoutAnimation(winrt::Microsoft::ReactNative::JSValue &&config) noexcept;
  void OnComplete(int64_t tag) noexcept;
  void Reset() noexcept;
  void SetHost(INativeUIManagerHost *host) noexcept;
  void AnimateLayoutProps(ShadowNodeBase &node, float left, float top, float width, float height);
  void SetLayoutProps(int64_t tag, float left, float top, float width, float height);
  bool ShouldAnimateLayout(XamlView const &view);

 private:
  bool m_shouldAnimateLayout{false};
  PaperUpdateLayoutAnimation m_updateLayoutAnimation{};
  INativeUIManagerHost *m_host{nullptr};
#ifndef USE_WINUI3
  std::unordered_map<int64_t, std::vector<AnimatedLayoutOperation>> m_tagsToOperations{};
#endif
};

}; // namespace Microsoft::ReactNative
