// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <INativeUIManager.h>
#include <Views/ShadowNodeBase.h>
#include <folly/dynamic.h>
#ifndef USE_WINUI3
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#endif

namespace react::uwp {

enum class InterpolatorType {
  Linear = 0,
  EaseIn,
  EaseOut,
  EaseInEaseOut,
  Spring,
};

// TODO: support create/drop view animations
class PaperLayoutAnimation {
 public:
  void ConfigureLayoutAnimation(folly::dynamic &config, int64_t globalDuration) noexcept;
  void Reset() noexcept;
#ifndef USE_WINUI3
  virtual xaml::Media::Animation::Storyboard CreateAnimation(
      ShadowNodeBase &node,
      float left,
      float top,
      float width,
      float height,
      std::function<void()> &&onComplete) = 0;
#endif

 protected:
  int64_t m_delay{};
  int64_t m_duration{};

  // TODO(T102475058): Remove Windows-specific spring config from LayoutAnimation
  std::optional<int64_t> m_springiness{};
  std::optional<int64_t> m_oscillations{};

#ifndef USE_WINUI3
  xaml::Media::Animation::EasingFunctionBase Interpolator() const;
#endif
 private:
  InterpolatorType m_type{};
};

}; // namespace react::uwp
