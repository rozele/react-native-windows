// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "PaperLayoutAnimation.h"
#include <Views/ViewManagerBase.h>

namespace Microsoft::ReactNative {

static std::unordered_map<std::string, InterpolatorType> s_interpolatorTypes{
  {"linear", InterpolatorType::Linear},
  {"easein", InterpolatorType::EaseIn},
  {"easeout", InterpolatorType::EaseOut},
  {"easeineaseout", InterpolatorType::EaseInEaseOut},
  {"spring", InterpolatorType::Spring},
};

void PaperLayoutAnimation::ConfigureLayoutAnimation(winrt::Microsoft::ReactNative::JSValue const &config, int globalDuration) noexcept {
  if (config.IsNull())
    return;

  m_delay = config["delay"].AsInt64();
  const auto &durationProperty = config["duration"];
  m_duration = !durationProperty.IsNull() ? durationProperty.AsInt64() : globalDuration;

  const auto type = s_interpolatorTypes.find(config["type"].AsString());
  if (type != s_interpolatorTypes.end()) {
    m_type = type->second;
  }
}

void PaperLayoutAnimation::Reset() noexcept {
  m_delay = {};
  m_duration = {};
  m_type = {};
}

void PaperLayoutAnimation::SetHost(INativeUIManagerHost *host) noexcept {
  m_host = host;
}

#ifndef USE_WINUI3
xaml::Media::Animation::EasingFunctionBase PaperLayoutAnimation::Interpolator() const {
  switch (m_type) {
    case InterpolatorType::EaseIn: {
    xaml::Media::Animation::QuadraticEase easeIn;
    easeIn.EasingMode(xaml::Media::Animation::EasingMode::EaseIn);
    return easeIn;
  }
  case InterpolatorType::EaseOut: {
    xaml::Media::Animation::QuadraticEase easeOut;
    easeOut.EasingMode(xaml::Media::Animation::EasingMode::EaseOut);
    return easeOut;
  }
  case InterpolatorType::EaseInEaseOut: {
    xaml::Media::Animation::QuadraticEase easeInEaseOut;
    easeInEaseOut.EasingMode(xaml::Media::Animation::EasingMode::EaseInOut);
    return easeInEaseOut;
  }
  case InterpolatorType::Spring: {
    xaml::Media::Animation::ElasticEase spring;
    spring.Springiness(10);
    spring.Oscillations(3);
    // TODO: convert springDamping to Springiness
    return spring;
  }
  default:
    return nullptr;
  }
}
#endif



}; // namespace Microsoft::ReactNative
