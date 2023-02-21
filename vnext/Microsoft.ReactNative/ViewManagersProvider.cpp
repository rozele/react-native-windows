// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "ViewManagersProvider.h"

#include "IReactContext.h"

#include "ABIViewManager.h"

#ifdef USE_WINUI_FABRIC
#include "ReactHost/MsoUtils.h"
#endif

namespace winrt::Microsoft::ReactNative {

/*-------------------------------------------------------------------------------
        ViewManagersProvider::GetViewManagers
-------------------------------------------------------------------------------*/
std::vector<std::unique_ptr<::Microsoft::ReactNative::IViewManager>> ViewManagersProvider::GetViewManagers(
    Mso::CntPtr<Mso::React::IReactContext> const &reactContext) {
  std::vector<std::unique_ptr<::Microsoft::ReactNative::IViewManager>> viewManagers;

  for (auto &entry : m_viewManagerProviders) {
    auto viewManagerProvider = entry.second;

    auto viewManager = std::make_unique<ABIViewManager>(reactContext, viewManagerProvider());

    viewManagers.emplace_back(std::move(viewManager));
  }

  return viewManagers;
}

#ifdef USE_WINUI_FABRIC
/*-------------------------------------------------------------------------------
        ViewManagersProvider::GetViewManagerInterfaces
-------------------------------------------------------------------------------*/
const std::vector<IViewManager> ViewManagersProvider::GetViewManagerInterfaces(
    Mso::CntPtr<Mso::React::IReactContext> const &reactContext) {
  std::vector<IViewManager> viewManagers;

  for (auto &entry : m_viewManagerProviders) {
    auto viewManagerProvider = entry.second;

    // TODO(T146190260): do we need a single view manager instance across both Paper and Fabric UIManagers?
    auto viewManager = viewManagerProvider();
    auto viewManagerWithReactContext = viewManager.try_as<IViewManagerWithReactContext>();
    if (viewManagerWithReactContext) {
      viewManagerWithReactContext.ReactContext(winrt::make<implementation::ReactContext>(Mso::Copy(reactContext)));
    }

    viewManagers.push_back(viewManager);
  }

  return viewManagers;
}
#endif

ViewManagersProvider::ViewManagersProvider() noexcept {}

void ViewManagersProvider::AddViewManagerProvider(
    winrt::hstring const &viewManagerName,
    ReactViewManagerProvider const &viewManagerProvider) noexcept {
  m_viewManagerProviders.emplace(to_string(viewManagerName), viewManagerProvider);
}

} // namespace winrt::Microsoft::ReactNative
