// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <Microsoft.ReactNative.Cxx/ReactContext.h>
#include <winrt/Microsoft.ReactNative.h>
#include "ComponentView.h"

namespace Microsoft::ReactNative {

struct ComponentViewDescriptor final {
  std::shared_ptr<IComponentView> view{nullptr};
  facebook::react::ComponentHandle componentHandle{};
};

/* This could be expanded to have a pool of ComponentViewDescriptor's, like iOS does */
class ComponentViewRegistry final {
 public:
  void Initialize(winrt::Microsoft::ReactNative::ReactContext const &reactContext) noexcept;

  ComponentViewDescriptor const &dequeueComponentViewWithComponentHandle(
      facebook::react::ComponentHandle componentHandle,
      facebook::react::Tag tag) noexcept;
  ComponentViewDescriptor const &componentViewDescriptorWithTag(facebook::react::Tag tag) const noexcept;
  std::shared_ptr<IComponentView> findComponentViewWithTag(facebook::react::Tag tag) const noexcept;
  void enqueueComponentViewWithComponentHandle(
      facebook::react::ComponentHandle componentHandle,
      facebook::react::Tag tag,
      ComponentViewDescriptor componentViewDescriptor) noexcept;
  void registerLegacyABIViewManager(
      facebook::react::ComponentHandle const &handle,
      winrt::Microsoft::ReactNative::IViewManager const &viewManager) noexcept;

 private:
  winrt::Microsoft::ReactNative::ReactContext m_context;
  std::unordered_map<facebook::react::Tag, ComponentViewDescriptor> m_registry;
  std::unordered_map<facebook::react::ComponentHandle, winrt::Microsoft::ReactNative::IViewManager>
      m_legacyABIViewManagers;
};

} // namespace Microsoft::ReactNative
