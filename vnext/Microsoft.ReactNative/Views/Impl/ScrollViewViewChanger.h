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

  bool OnSizeChanged(const xaml::Controls::ScrollViewer &scrollViewer);
  bool OnViewChanging(
      const xaml::Controls::ScrollViewer &scrollViewer,
      const xaml::Controls::ScrollViewerViewChangingEventArgs &args);
  void OnViewChanged(const xaml::Controls::ScrollViewerViewChangedEventArgs &args);

 private:
  bool m_inverted{false};
  bool m_horizontal{false};

  bool m_activeScrollCommand{false};

  winrt::IReference<double> m_lastChangeViewX{nullptr};
  winrt::IReference<double> m_lastChangeViewY{nullptr};
  bool m_lastChangeViewAnimated{false};

  double m_latestOnScrollX{0};
  double m_latestOnScrollY{0};

  bool UpdateLatestOffsets(const xaml::Controls::ScrollViewer &scrollViewer, double x, double y);
  static void SetContentScrollAnchors(const xaml::Controls::ScrollViewer &scrollViewer, bool enabled);
};

} // namespace react::uwp
