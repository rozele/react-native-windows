// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace react::uwp {

class ScrollViewViewChanger {
 public:
  double OffsetEpsilon() const;
  void Horizontal(bool horizontal);
  bool Inverted() const;
  void Inverted(bool inverted);

  std::tuple<double, double> GetScrollOffsets(xaml::Controls::ScrollViewer scrollViewer, double x, double y);

  void ChangeView(
      xaml::Controls::ScrollViewer scrollViewer,
      const winrt::IReference<double> x,
      const winrt::IReference<double> y,
      bool disableAnimated);

  void OnSizeChanged(xaml::Controls::ScrollViewer scrollViewer);
  void OnViewChanged(xaml::Controls::ScrollViewerViewChangedEventArgs args);
  void OnViewChanging(xaml::Controls::ScrollViewerViewChangingEventArgs args);

 private:
  bool m_inverted{false};
  bool m_horizontal{false};

  bool m_activeScrollCommand{false};
  double m_adjustedTargetX;
  double m_adjustedTargetY;

  winrt::IReference<double> m_lastX;
  winrt::IReference<double> m_lastY;
  bool m_lastAnimated{false};
};

} // namespace react::uwp
