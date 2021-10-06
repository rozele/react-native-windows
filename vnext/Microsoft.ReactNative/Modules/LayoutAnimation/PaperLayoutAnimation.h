// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <JSValue.h>
#include <ShadowNodeBase.h>
#include <INativeUIManager.h>
#ifndef USE_WINUI3
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#endif

namespace Microsoft::ReactNative {

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
  void ConfigureLayoutAnimation(winrt::Microsoft::ReactNative::JSValue const &config, int globalDuration) noexcept;
  void Reset() noexcept;
  void SetHost(INativeUIManagerHost *host) noexcept;
#ifndef USE_WINUI3
  virtual xaml::Media::Animation::Storyboard CreateAnimation(ShadowNodeBase &node, float left, float top, float width, float height, std::function<void()> &&onComplete) = 0;
#endif

 protected:
  int64_t m_delay{};
  int64_t m_duration{};

  xaml::Media::Animation::EasingFunctionBase Interpolator() const;
 private:
  InterpolatorType m_type{};
  INativeUIManagerHost *m_host{nullptr};
};

}; // namespace Microsoft::ReactNative
