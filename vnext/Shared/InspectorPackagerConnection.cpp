// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "pch.h"

#include <Shared/DevServerHelper.h>
#include <Shared/DevSettings.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Networking.Sockets.h>
#include <winrt/Windows.Storage.Streams.h>
#include "Unicode.h"
#include "Utilities.h"

#pragma warning(push)
#pragma warning(disable : 4146 4244 4068 4251 4101 4267 4804 4309)
#include <cxxreact/JSExecutor.h>
#include <jsinspector/InspectorInterfaces.h>
#pragma warning(pop)

#include <assert.h>
#include <atomic>
#include <chrono>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <future>
#include <mutex>
#include <unordered_map>

#if _MSC_VER <= 1913
// VC 19 (2015-2017.6) cannot optimize co_await/cppwinrt usage
#pragma optimize("", off)
#endif

namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
} // namespace winrt

namespace facebook::react {

class InspectorPackagerConnection
    : public std::enable_shared_from_this<InspectorPackagerConnection> {
 public:
  InspectorPackagerConnection(const std::string& url);
  ~InspectorPackagerConnection();
  winrt::IAsyncAction Connect();
  void Close();

 private:
  winrt::IAsyncAction DelayAndReconnect();
  winrt::IAsyncAction SendMessageAsync(const std::string& message);
  void OnMessageReceived(const folly::dynamic& message);
  void SendEvent(const std::string& name, const folly::dynamic& payload);
  void SendWrappedEvent(const std::string& pageId, const std::string& message);
  void HandleWrappedEvent(const folly::dynamic& payload);
  void HandleConnect(const folly::dynamic& payload);
  void HandleDisconnect(const folly::dynamic& payload);
  folly::dynamic MakePageIdPayload(const std::string& pageId);
  static IInspector* GetInspector() {
    return &facebook::react::getInspectorInstance();
  }
  folly::dynamic GetPages();
  void CloseConnections();
  void OnError(const char* error);
  void OnConnectionError(int socketRev, const char* error);

  // We need to be able to reference and use a local connection outside the lock
  // for connections - wrap a shared_ptr around the unique_ptr interface.
  struct LocalConnection {
    LocalConnection(std::unique_ptr<ILocalConnection>&& connection)
        : local(std::move(connection)) {}
    ~LocalConnection() {
      if (local) {
        local->disconnect();
      }
    }
    std::unique_ptr<ILocalConnection> local;
  };

  std::shared_ptr<LocalConnection> GetLocalConnection(
      const std::string& pageId,
      bool remove = false);
  void RemoveLocalConnection(const std::string& pageId);

  struct RemoteConnection : IRemoteConnection {
    RemoteConnection(
        std::weak_ptr<InspectorPackagerConnection> parent,
        const std::string& pageId)
        : m_pageId(pageId), m_parent(parent) {}
    ~RemoteConnection() {}

    void onMessage(std::string message) override {
      if (auto parent = m_parent.lock()) {
        parent->SendWrappedEvent(m_pageId, message);
      }
    }

    // Remote connection's disconnect may get posted to another thread.
    void onDisconnect() override {
      if (auto parent = m_parent.lock()) {
        // Make sure that page is no longer in our connection list. If we are
        // cleaning up the remote connection, we don't want to touch the local
        // connection any more.
        auto connection = parent->GetLocalConnection(m_pageId, /*remove*/ true);
        if (connection) {
          connection->local.reset();
        }
        auto payload = parent->MakePageIdPayload(m_pageId);
        parent->SendEvent("disconnect", payload);
      }
    }

    std::weak_ptr<InspectorPackagerConnection> m_parent;
    std::string m_pageId;
  };

  struct WebSocket {
    winrt::Windows::Networking::Sockets::MessageWebSocket socket;
    winrt::Windows::Storage::Streams::DataWriter socketWriter = nullptr;
    winrt::event_revoker<winrt::IMessageWebSocket> received;
    winrt::event_revoker<winrt::IWebSocket> closed;
    int rev = 0;
  };

  std::mutex m_connectionsMutex;
  std::unordered_map<std::string, std::shared_ptr<LocalConnection>>
      m_inspectorConnections;
  std::mutex m_socketMutex;
  std::shared_ptr<WebSocket> m_socket;
  bool m_closed = false;
  std::atomic<int> m_nextSocketRev = 1;
  std::string m_url;
  int m_errors = 0;
  const char* m_lastError;
  bool m_reconnecting = false;
  Mso::DispatchQueue m_queue = nullptr;
};

InspectorPackagerConnection::InspectorPackagerConnection(const std::string& url)
    : m_url(url) {
  m_queue = Mso::DispatchQueue::MakeLooperQueue();
}

std::shared_ptr<InspectorPackagerConnection> InspectorPackagerConnectionCreate(
    const std::string& url) {
  auto connection = std::make_shared<InspectorPackagerConnection>(url);
  // Kick off the asynchronous connection attempt.
  connection->Connect();
  return connection;
}

winrt::IAsyncAction InspectorPackagerConnection::Connect() {
  winrt::Windows::Foundation::Uri uri(
      Microsoft::Common::Unicode::Utf8ToUtf16(m_url));

  auto socket = std::make_shared<WebSocket>();
  socket->rev = m_nextSocketRev++;
  socket->socket.Control().MessageType(winrt::SocketMessageType::Utf8);
  socket->socketWriter = winrt::DataWriter(socket->socket.OutputStream());
  socket->received = socket->socket.MessageReceived(
      winrt::auto_revoke, [weak = weak_from_this(), this, rev = socket->rev](auto&&, auto&& args) {
        auto strong = weak.lock();
        if (!strong) {
          return;
        }
        try {
          std::string response;
          if (args.MessageType() == winrt::SocketMessageType::Utf8) {
            winrt::DataReader reader = args.GetDataReader();
            reader.UnicodeEncoding(winrt::UnicodeEncoding::Utf8);
            uint32_t len = reader.UnconsumedBufferLength();
            std::vector<uint8_t> data(len);
            reader.ReadBytes(data);
            std::string str(reinterpret_cast<char*>(data.data()), data.size());
            auto message = folly::parseJson(str);
            const auto& event = message["event"];
            if (event.isString()) {
              m_queue.Post([weak = weak_from_this(), this, msg = std::move(message)]() {
                if (auto strong = weak.lock()) {
                  OnMessageReceived(msg);
                }
              });
            }
          } else {
            OnError("MessageReceived: not utf8");
          }
        } catch (folly::TypeError const& e) {
          OnError("MessageReceived: type error");
        } catch (folly::json::parse_error const& e) {
          OnError("MessageReceived: parse error");
        } catch (winrt::hresult_error const& e) {
          auto hr = e.code();
          if (hr == WININET_E_CONNECTION_ABORTED ||
              hr == WININET_E_CONNECTION_RESET) {
            OnConnectionError(rev, "MessageReceived: connection failed");
          } else {
            OnConnectionError(rev, "MessageReceived: other winrt");
          }
        } catch (std::exception& e) {
          OnConnectionError(rev, "MessageReceived: std error");
        }
      });

  socket->closed = socket->socket.Closed(
      winrt::auto_revoke, [weak = weak_from_this(), this, rev = socket->rev](auto&&, auto&& args) {
        auto strong = weak.lock();
        if (!strong) {
          return;
        }
        OnConnectionError(rev, "SocketClosed");
      });

  {
    std::lock_guard<std::mutex> lock(m_socketMutex);
    // There should be one connection/reconnection attempt outstanding.
    assert(!m_socket);
    if (m_closed) {
      co_return;
    }
    m_socket = socket;
  }

  // Make sure "this" will live as we are trying to connect, so we can handle errors etc.
  auto strong = shared_from_this();
  try {
    co_await socket->socket.ConnectAsync(uri);
    // We will come back on some arbitrary thread from co_await.
  } catch (...) { // Clang bug, cannot catch exception type here:
                  // https://reviews.llvm.org/D33733
    OnConnectionError(socket->rev, "ConnectAsync failed");
  }
}

void InspectorPackagerConnection::OnMessageReceived(
    const folly::dynamic& message) {
  const auto& event = message["event"].asString();
  if (event == "getPages") {
    SendEvent("getPages", GetPages());
  } else if (event == "wrappedEvent") {
    HandleWrappedEvent(message["payload"]);
  } else if (event == "connect") {
    HandleConnect(message["payload"]);
  } else if (event == "disconnect") {
    HandleDisconnect(message["payload"]);
  } else {
    OnError("Unsupported event");
  }
}

void InspectorPackagerConnection::SendEvent(
    const std::string& name,
    const folly::dynamic& payload) {
  folly::dynamic message =
      folly::dynamic::object("event", name)("payload", payload);
  SendMessageAsync(folly::toJson(message));
}

void InspectorPackagerConnection::SendWrappedEvent(
    const std::string& pageId,
    const std::string& message) {
  folly::dynamic payload =
      folly::dynamic::object("pageId", pageId)("wrappedEvent", message);
  SendEvent("wrappedEvent", payload);
}

winrt::IAsyncAction InspectorPackagerConnection::SendMessageAsync(
    const std::string& message) {
  // Make sure "this" will live as we are trying to send, so we can handle errors etc.
  auto strong = shared_from_this();
  int rev = 0;
  try {
    winrt::DataWriterStoreOperation storeOp = nullptr;
    {
      // We do not write out message at a time even if we are trying to send
      // from two different connections/threads.
      std::lock_guard<std::mutex> lock(m_socketMutex);
      if (!m_socket) {
        co_return;
      }
      rev = m_socket->rev;
      winrt::array_view<const uint8_t> arr(
          reinterpret_cast<const uint8_t*>(message.c_str()),
          reinterpret_cast<const uint8_t*>(message.c_str()) + message.length());
      m_socket->socketWriter.WriteBytes(arr);
      storeOp = m_socket->socketWriter.StoreAsync();
    }
    co_await storeOp;
    // We will come back on some arbitrary thread from co_await.
  } catch (...) { // Clang bug, cannot catch exception type here:
                  // https://reviews.llvm.org/D33733
    OnConnectionError(rev, "SendMessageAync failure");
  }
}

folly::dynamic InspectorPackagerConnection::GetPages() {
  const std::vector<InspectorPage>& pages = GetInspector()->getPages();
  folly::dynamic jsonPages = folly::dynamic::array;
  for (const auto& page : pages) {
    folly::dynamic jsonPage =
        folly::dynamic::object("id", std::to_string(page.id))(
            "title", page.title)("app", "JacksApp") // FAKE
        ("vm", page.vm)("isLastBundleDownloadSuccess", nullptr)(
            "bundleUpdateTimestamp", nullptr);
    jsonPages.push_back(std::move(jsonPage));
  }
  return jsonPages;
}

std::shared_ptr<InspectorPackagerConnection::LocalConnection>
InspectorPackagerConnection::GetLocalConnection(
    const std::string& pageId,
    bool remove) {
  {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    auto found = m_inspectorConnections.find(pageId);
    if (found == m_inspectorConnections.end()) {
      return nullptr;
    }
    auto connection = found->second;
    if (remove) {
      m_inspectorConnections.erase(found);
    }
    return connection;
  }
}

void InspectorPackagerConnection::RemoveLocalConnection(
    const std::string& pageId) {
  // When the last reference to the local connection is released, it will
  // disconnect. We don't want to disconnnect it while someone may be using it
  // though, that's why we have shared_ptr.
  auto connection = GetLocalConnection(pageId, /*remove*/ true);
}

void InspectorPackagerConnection::HandleWrappedEvent(
    const folly::dynamic& payload) {
  const auto& pageId = payload["pageId"].asString();
  const auto& wrappedEvent = payload["wrappedEvent"].asString();
  auto connection = GetLocalConnection(pageId);
  if (connection) {
    connection->local->sendMessage(wrappedEvent);
  }
}

void InspectorPackagerConnection::HandleDisconnect(
    const folly::dynamic& payload) {
  const auto& pageId = payload["pageId"].asString();
  RemoveLocalConnection(pageId);
}

void InspectorPackagerConnection::HandleConnect(const folly::dynamic& payload) {
  const auto& pageId = payload["pageId"].asString();
  if (GetLocalConnection(pageId)) {
    OnError("Page already connected");
    RemoveLocalConnection(pageId);
  }

  // Fortunately connections are only made on the queue, so we know no one else
  // can raced with us to insert a new connection for this page.
  std::unique_ptr<IRemoteConnection> remote{
      new RemoteConnection(weak_from_this(), pageId)};
  auto localConnection =
      GetInspector()->connect(std::stoi(pageId), std::move(remote));
  if (!localConnection) {
    OnError("Failed inspector local connection");
    return;
  }

  auto connection =
      std::make_shared<LocalConnection>(std::move(localConnection));
  {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_inspectorConnections.insert(std::make_pair(pageId, connection));
  }
}

folly::dynamic InspectorPackagerConnection::MakePageIdPayload(
    const std::string& pageId) {
  return folly::dynamic::object("pageId", pageId);
}

void InspectorPackagerConnection::OnError(const char* error) {
  m_errors++;
  m_lastError = error;
}

void InspectorPackagerConnection::OnConnectionError(int socketRev, const char* error) {
  // Note: We will most likely get called on an arbitrary networking thread.
  // If we have closed the connection, ignore any errors due to it.
  if (m_closed) {
    return;
  }

  OnError(error);

  std::shared_ptr<WebSocket> socket;
  bool closed = false;
  {
    std::lock_guard<std::mutex> lock(m_socketMutex);
    if (!m_socket || m_socket->rev != socketRev) {
      // The error is not for the current socket, or we've already kicked off reconnect.
      return;
    }
    // Socket will be destructed outside the lock.
    m_socket.swap(socket);
    closed = m_closed;
  }

  // If we are not winding down, we'll try to reconnect. We'll start on the
  // queue by disconnecting all connections.
  if (!closed) {
    m_queue.Post([weak = weak_from_this(), this]() {
      if (auto strong = weak.lock()) {
        CloseConnections();
        DelayAndReconnect();
      }
    });
  }
}

void InspectorPackagerConnection::CloseConnections() {
  // Make sure the local connections will get disconnected outside the lock when
  // LocalConnection destructors are called.
  std::unordered_map<std::string, std::shared_ptr<LocalConnection>> connections;
  {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_inspectorConnections.swap(connections);
  }
  connections.clear();
}

winrt::IAsyncAction InspectorPackagerConnection::DelayAndReconnect() {
  auto weak = weak_from_this();
  // For syntax: https://devblogs.microsoft.com/oldnewthing/20191219-00/?p=103230
  using namespace std::chrono_literals;
  co_await winrt::operator co_await(2s);
  // We will come back from co_await on some arbitrary thread. Our object may have
  // been deleted while we are waiting, e.g. a dev initiated reload.
  if (auto strong = weak.lock()) {
    Connect();
  }
}

void InspectorPackagerConnectionClose(InspectorPackagerConnection* connection) {
  connection->Close();
}

void InspectorPackagerConnection::Close() {
  // This should help wind everything down quicker.
  std::shared_ptr<WebSocket> socket;
  {
    std::lock_guard<std::mutex> lock(m_socketMutex);
    m_closed = true;
    m_socket.swap(socket);
  }
}

InspectorPackagerConnection::~InspectorPackagerConnection() {
  // We shutdown the queue but do not wait for it to be flushed out.
  m_queue.Shutdown(Mso::PendingTaskAction::Complete);

  // Drop the local connections without disconnecting. If a debugger was
  // connected, on reload this leaves a local connection registered which will
  // make Hermes wait for the debugger, allowing us to debug early JS.
  const std::string c_debuggerDisable = "{ \"id\":1,\"method\":\"Debugger.disable\" }";
  for (auto& entry : m_inspectorConnections) {
    auto& local = entry.second->local;
    // Make sure no one is stuck in the debugger.
    local->sendMessage(c_debuggerDisable);
    local.reset();
  }
}

} // namespace facebook::react
