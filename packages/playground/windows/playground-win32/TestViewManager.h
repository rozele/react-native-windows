#pragma once

#include <NativeModules.h>
#include <winrt/Microsoft.ReactNative.h>

namespace playground {

namespace rn {
using namespace winrt::Microsoft::ReactNative;
}

struct TestViewManager : winrt::implements<struct TestViewManager,
                                 rn::IViewManager,
                                 rn::IViewManagerWithNativeProperties,
                                 rn::IViewManagerWithNativeLayout> {
 public:
  TestViewManager() = default;

  // IViewManager
  winrt::hstring Name() noexcept;
  winrt::Windows::UI::Xaml::FrameworkElement CreateView() noexcept;

  // IViewManagerWithNativeProperties
  winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, rn::ViewManagerPropertyType> NativeProps() noexcept;

  void UpdateProperties(
      winrt::Windows::UI::Xaml::FrameworkElement const &view,
      rn::IJSValueReader const &propertyMapReader) noexcept;

  // IViewManagerWithNativeLayout
  rn::YogaSize Measure(
      xaml::FrameworkElement view,
      float width,
      rn::YogaMeasureMode widthMode,
      float height,
      rn::YogaMeasureMode heightMode) noexcept;
};

} // namespace playground
