#pragma once

#include <winrt/Microsoft.ReactNative.h>

namespace playground {

struct PlaygroundReactPackageProvider
    : winrt::implements<PlaygroundReactPackageProvider, winrt::Microsoft::ReactNative::IReactPackageProvider> {
  void CreatePackage(winrt::Microsoft::ReactNative::IReactPackageBuilder const &packageBuilder) noexcept;
};

} // playground
