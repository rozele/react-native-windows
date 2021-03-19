// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace react::uwp {

class ScrollViewViewChanger {
 public:
  void Horizontal(bool horizontal);
  bool Inverted() const;
  void Inverted(bool inverted);

  std::tuple<double, double> GetScrollOffsets(xaml::Controls::ScrollViewer scrollViewer, double x, double y);

  void ChangeView(
      xaml::Controls::ScrollViewer scrollViewer,
      const winrt::IReference<double> x,
      const winrt::IReference<double> y,
      bool disableAnimated);

  void OnSizeChanged(const xaml::Controls::ScrollViewer &scrollViewer);
  void OnViewChanging(
      const xaml::Controls::ScrollViewer &scrollViewer,
      const xaml::Controls::ScrollViewerViewChangingEventArgs &args);
  void OnViewChanged(const xaml::Controls::ScrollViewerViewChangedEventArgs &args);

 private:
  bool m_inverted{false};
  bool m_horizontal{false};

  bool m_activeScrollCommand{false};
  double m_adjustedTargetX{0};
  double m_adjustedTargetY{0};

  winrt::IReference<double> m_lastX{nullptr};
  winrt::IReference<double> m_lastY{nullptr};
  bool m_lastAnimated{false};

  double m_latestX{0};
  double m_latestY{0};

  static void SetContentScrollAnchors(const xaml::Controls::ScrollViewer &scrollViewer, bool enabled);
};

} // namespace react::uwp
