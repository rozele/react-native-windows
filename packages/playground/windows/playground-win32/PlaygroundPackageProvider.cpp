#include "pch.h"
#include "PlaygroundPackageProvider.h"
#include "TestViewManager.h"

using namespace winrt::Microsoft::ReactNative;
namespace playground {

void PlaygroundReactPackageProvider::CreatePackage(IReactPackageBuilder const &packageBuilder) noexcept {
  packageBuilder.AddViewManager(L"TestViewManager", [] { return winrt::make<TestViewManager>(); });
}
} // namespace playground
