// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "PaperLayoutAnimation.h"
#include <Views/ViewManagerBase.h>

namespace react::uwp {

static std::unordered_map<std::string, InterpolatorType> s_interpolatorTypes{
    {"linear", InterpolatorType::Linear},
    {"easein", InterpolatorType::EaseIn},
    {"easeout", InterpolatorType::EaseOut},
    {"easeineaseout", InterpolatorType::EaseInEaseOut},
    {"spring", InterpolatorType::Spring},
};

void PaperLayoutAnimation::ConfigureLayoutAnimation(folly::dynamic &config, int64_t globalDuration) noexcept {
  if (config == nullptr)
    return;

  m_delay = config["delay"].asInt();
  const auto &durationProperty = config["duration"];
  m_duration = durationProperty != nullptr ? durationProperty.asInt() : globalDuration;

  const auto type = s_interpolatorTypes.find(config["type"].asString());
  if (type != s_interpolatorTypes.end()) {
    m_type = type->second;
  }

  const auto &springinessProperty = config["springiness"];
  m_springiness = springinessProperty != nullptr ? std::optional(springinessProperty.asInt()) : std::nullopt;

  const auto &oscillationsProperty = config["oscillations"];
  m_oscillations = oscillationsProperty != nullptr ? std::optional(oscillationsProperty.asInt()) : std::nullopt;
}

void PaperLayoutAnimation::Reset() noexcept {
  m_delay = {};
  m_duration = {};
  m_type = {};
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
      // TODO(T102475058): Remove Windows-specific spring config from LayoutAnimation
      if (m_springiness) {
        spring.Springiness(m_springiness.value());
      }
      if (m_oscillations) {
        spring.Springiness(m_oscillations.value());
      }
      return spring;
    }
    default:
      return nullptr;
  }
}
#endif

}; // namespace react::uwp
