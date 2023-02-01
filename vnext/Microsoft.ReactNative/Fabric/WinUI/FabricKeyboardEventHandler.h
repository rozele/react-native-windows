// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include <IReactInstance.h>
#include <JSValue.h>
#include <React.h>
#include <folly/dynamic.h>
#include <react/renderer/components/view/windows/KeyEvent.h>
#include <optional>
#include <set>
#include "CppWinRTIncludes.h"
#include "XamlView.h"

namespace winrt {
using namespace Windows::UI::Core;
} // namespace winrt

namespace Microsoft::ReactNative {
enum class FabricKeyEventType { Up, Down };

typedef std::function<void(winrt::IInspectable const &, xaml::Input::KeyRoutedEventArgs const &)>
    FabricKeyboardEventCallback;

class FabricKeyboardEventBaseHandler {
 public:
  FabricKeyboardEventBaseHandler(FabricKeyboardEventCallback &&keyDown, FabricKeyboardEventCallback &&keyUp);
  virtual ~FabricKeyboardEventBaseHandler() = default;

  virtual void hook(XamlView xamlView) = 0;
  virtual void unhook() = 0;

 protected:
  FabricKeyboardEventCallback m_keyDownCallback;
  FabricKeyboardEventCallback m_keyUpCallback;
};

class FabricPreviewKeyboardEventHandler : public FabricKeyboardEventBaseHandler {
 public:
  FabricPreviewKeyboardEventHandler(FabricKeyboardEventCallback &&keyDown, FabricKeyboardEventCallback &&keyUp);

  void hook(XamlView xamlView);
  void unhook();

 private:
  xaml::UIElement::PreviewKeyDown_revoker m_previewKeyDownRevoker{};
  xaml::UIElement::PreviewKeyUp_revoker m_previewKeyUpRevoker{};
};

class FabricKeyboardEventHandler : public FabricKeyboardEventBaseHandler {
 public:
  FabricKeyboardEventHandler(FabricKeyboardEventCallback &&keyDown, FabricKeyboardEventCallback &&keyUp);

  void hook(XamlView xamlView);
  void unhook();

 private:
  xaml::UIElement::KeyDown_revoker m_keyDownRevoker{};
  xaml::UIElement::KeyUp_revoker m_keyUpRevoker{};
};

class FabricPreviewKeyboardEventHandlerOnRoot : public FabricPreviewKeyboardEventHandler {
 public:
  FabricPreviewKeyboardEventHandlerOnRoot(const Mso::React::IReactContext &context);

 private:
  void OnPreKeyDown(winrt::IInspectable const &sender, xaml::Input::KeyRoutedEventArgs const &args);
  void OnPreKeyUp(winrt::IInspectable const &sender, xaml::Input::KeyRoutedEventArgs const &args);

  void DispatchEvent(FabricKeyEventType const &type, xaml::Input::KeyRoutedEventArgs const &args);
  Mso::CntPtr<const Mso::React::IReactContext> m_context;
};

class FabricHandledKeyboardEventHandler {
 public:
  enum class KeyboardEventPhase { PreviewKeyUp, PreviewKeyDown, KeyUp, KeyDown };

  FabricHandledKeyboardEventHandler();

  void hook(XamlView xamlView);
  void unhook();

 public:
  void UpdateHandledKeyboardEvents(
      FabricKeyEventType const &type,
      std::vector<facebook::react::HandledKeyEvent> const &handledKeyEvent);

 private:
  void EnsureKeyboardEventHandler();

  void KeyboardEventHandledHandler(
      KeyboardEventPhase phase,
      winrt::IInspectable const &sender,
      xaml::Input::KeyRoutedEventArgs const &args);

  std::vector<facebook::react::HandledKeyEvent> m_handledKeyUpKeyboardEvents;
  std::vector<facebook::react::HandledKeyEvent> m_handledKeyDownKeyboardEvents;

  std::unique_ptr<FabricPreviewKeyboardEventHandler> m_previewKeyboardEventHandler;
  std::unique_ptr<FabricKeyboardEventHandler> m_keyboardEventHandler;
};

struct FabricKeyboardHelper {
  static facebook::react::HandledKeyEvent CreateKeyboardEvent(
      facebook::react::HandledEventPhase phase,
      xaml::Input::KeyRoutedEventArgs const &args);
  static std::string FromVirtualKey(winrt::Windows::System::VirtualKey key, bool shiftDown, bool capLocked);
  static std::string CodeFromVirtualKey(winrt::Windows::System::VirtualKey key);
  static bool IsModifiedKeyPressed(winrt::CoreWindow const &coreWindow, winrt::Windows::System::VirtualKey virtualKey);
  static bool IsModifiedKeyLocked(winrt::CoreWindow const &coreWindow, winrt::Windows::System::VirtualKey virtualKey);
  static bool ShouldMarkKeyboardHandled(
      std::vector<facebook::react::HandledKeyEvent> const &handledEvents,
      facebook::react::HandledKeyEvent currentEvent);
};
} // namespace Microsoft::ReactNative
