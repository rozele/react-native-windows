#include "pch.h"
#include "TestViewManager.h"
#include "JSValueXaml.h"
#include <UI.Xaml.Controls.h>
#include <UI.Xaml.Media.h>

namespace playground {

winrt::hstring TestViewManager::Name() noexcept {
  return L"TestView";
}

xaml::FrameworkElement TestViewManager::CreateView() noexcept {
  return xaml::Controls::Grid{};
}

winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, rn::ViewManagerPropertyType>
TestViewManager::NativeProps() noexcept {
  auto nativeProps = winrt::single_threaded_map<winrt::hstring, rn::ViewManagerPropertyType>();
  nativeProps.Insert(L"backgroundColor", rn::ViewManagerPropertyType::Color);
  return nativeProps.GetView();
}

void TestViewManager::UpdateProperties(
    xaml::FrameworkElement const &view,
    rn::IJSValueReader const &propertyMapReader) noexcept {
  //if (const auto grid = view.try_as<xaml::Controls::Grid>()) {
  //  rn::JSValueObject propertyMap = rn::JSValueObject::ReadFrom(propertyMapReader);
  //  for (const auto &pair : propertyMap) {
  //    const auto &propertyName = winrt::to_hstring(pair.first);
  //    const auto &propertyValue = pair.second;
  //    if (propertyName == L"backgroundColor") {
  //      //grid.Background(pair.second.To<xaml::Media::Brush>());
  //    }
  //  }
  //}
}

rn::YogaSize TestViewManager::Measure(
    xaml::FrameworkElement element,
    float width,
    rn::YogaMeasureMode widthMode,
    float height,
    rn::YogaMeasureMode heightMode) noexcept {
  float maxWidth = 1000;
  float maxHeight = 1000;
  if (widthMode != rn::YogaMeasureMode::Undefined) {
    maxWidth = std::min(maxWidth, width);
  }

  if (heightMode != rn::YogaMeasureMode::Undefined) {
    maxHeight = std::min(maxHeight, height);
  }

  return {maxWidth, maxHeight};
}

} // namespace playground
