// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <IDevSupportManager.h>

#include <DevServerHelper.h>

#include <winrt/Windows.Networking.Sockets.h>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <string>

namespace facebook {
namespace react {
struct DevSettings;
class InspectorPackagerConnection;
std::shared_ptr<InspectorPackagerConnection> InspectorPackagerConnectionCreate(const std::string& url);
void InspectorPackagerConnectionClose(InspectorPackagerConnection* connection);
}
} // namespace facebook

namespace Microsoft::ReactNative {

std::pair<std::string, bool> GetJavaScriptFromServer(
    const std::string &sourceBundleHost,
    const uint16_t sourceBundlePort,
    const std::string &jsBundleName,
    const std::string &platform,
    bool inlineSourceMap);

class DevSupportManager final : public facebook::react::IDevSupportManager {
 public:
  DevSupportManager() = default;
  ~DevSupportManager();

  virtual facebook::react::JSECreator LoadJavaScriptInProxyMode(const facebook::react::DevSettings &settings) override;
  virtual void StartPollingLiveReload(
      const std::string &sourceBundleHost,
      const uint16_t sourceBundlePort,
      std::function<void()> onChangeCallback) override;
  virtual void StopPollingLiveReload() override;
  virtual bool HasException() override {
    return m_exceptionCaught;
  }
  virtual void StartInspectorConnection(const facebook::react::DevSettings &settings) override;
  virtual void DisableInspectorDebugger() override;

 private:
  std::shared_ptr<facebook::react::InspectorPackagerConnection> m_inspectorConnection;
  bool m_exceptionCaught = false;
  std::atomic_bool m_cancellation_token;
};

} // namespace Microsoft::ReactNative
