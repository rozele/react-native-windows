// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentInterpolationAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"

namespace Microsoft::ReactNative {

DependentInterpolationAnimatedNode::DependentInterpolationAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager)
    : Super(tag, manager) {
  for (const auto &rangeValue : config.find("inputRange").dereference().second) {
    m_inputRanges.push_back(rangeValue.asDouble());
  }
  for (const auto &rangeValue : config.find("outputRange").dereference().second) {
    m_outputRanges.push_back(rangeValue.asDouble());
  }

  m_extrapolateLeft = config.find("extrapolateLeft").dereference().second.asString();
  m_extrapolateRight = config.find("extrapolateRight").dereference().second.asString();
}

void DependentInterpolationAnimatedNode::OnDetachedFromNode(int64_t animatedNodeTag) {
  assert(m_parentTag == animatedNodeTag);
  m_parentTag = s_parentTagUnset;
}

void DependentInterpolationAnimatedNode::OnAttachToNode(int64_t animatedNodeTag) {
  assert(m_parentTag == s_parentTagUnset);
  m_parentTag = animatedNodeTag;
}

static double Interpolate(
    double value,
    double inputMin,
    double inputMax,
    double outputMin,
    double outputMax,
    std::string_view extrapolateLeft,
    std::string_view extrapolateRight) {
  auto result = value;

  // Extrapolate
  if (result < inputMin) {
    if (extrapolateLeft == "identity") {
      return result;
    } else if (extrapolateLeft == "clamp") {
      result = inputMin;
    }
  }

  if (result > inputMax) {
    if (extrapolateRight == "identity") {
      return result;
    } else if (extrapolateRight == "clamp") {
      result = inputMax;
    }
  }

  return outputMin + (outputMax - outputMin) * (result - inputMin) / (inputMax - inputMin);
}

static int FindRangeIndex(double value, std::vector<double> const &ranges) {
  auto index = 1;
  for (; index < ranges.size() - 1; ++index) {
    if (ranges[index] >= value) {
      break;
    }
  }

  return index - 1;
}

static double Interpolate(
    double value,
    std::vector<double> const &inputRange,
    std::vector<double> const &outputRange,
    std::string_view extrapolateLeft,
    std::string_view extrapolateRight) {
  auto rangeIndex = FindRangeIndex(value, inputRange);
  return Interpolate(
      value,
      inputRange[rangeIndex],
      inputRange[rangeIndex + 1],
      outputRange[rangeIndex],
      outputRange[rangeIndex + 1],
      extrapolateLeft,
      extrapolateRight);
}

void DependentInterpolationAnimatedNode::Update() noexcept {
  if (m_parentTag == s_parentTagUnset) {
    return;
  }

  if (const auto manager = m_manager.lock()) {
    if (const auto node = manager->GetValueAnimatedNode(m_parentTag)) {
      RawValue(Interpolate(node->Value(), m_inputRanges, m_outputRanges, m_extrapolateLeft, m_extrapolateRight));
    }
  }
}

} // namespace Microsoft::ReactNative
