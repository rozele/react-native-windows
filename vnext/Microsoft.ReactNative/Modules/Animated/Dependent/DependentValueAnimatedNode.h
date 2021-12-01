// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"

namespace Microsoft::ReactNative {
class DependentAnimationDriver;
class DependentValueAnimatedNode : public DependentAnimatedNode {
  using Super = DependentAnimatedNode;
 public:
  DependentValueAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  DependentValueAnimatedNode(int64_t tag, const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  double Value() const noexcept {
    return m_value + m_offset;
  }
  double RawValue() const noexcept {
    return m_value;
  }
  void RawValue(double value) noexcept {
    m_value = value;
  }
  double Offset() const noexcept {
    return m_offset;
  }
  void Offset(double offset) noexcept {
    m_offset = offset;
  }
  void FlattenOffset() noexcept {
    m_value += m_offset;
    m_offset = 0;
  }
  void ExtractOffset() noexcept {
    m_offset += m_value;
    m_value = 0;
  }
  void OnValueUpdate() noexcept {
    m_valueChangedEvent(Value());
  }
  winrt::event_token ValueChanged(winrt::delegate<double> const& onValueChanged) {
    return m_valueChangedEvent.add(onValueChanged);
  }
  void ValueChanged(winrt::event_token const& token) {
    m_valueChangedEvent.remove(token);
  }

 private:
  double m_offset;
  double m_value;
  winrt::event<winrt::delegate<double>> m_valueChangedEvent;
};
} // namespace Microsoft::ReactNative
