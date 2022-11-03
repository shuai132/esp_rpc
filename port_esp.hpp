#pragma once

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

#include <cstdint>
#include <string>

namespace esp_rpc {

struct tcp_session {
  explicit tcp_session(AsyncClient* client) : client_(client) {
    client->onData([this](void*, AsyncClient*, void* data, size_t len) {
      if (on_data) on_data(std::string((char*)data, len));
    });
  }

  std::function<void()> on_close;

  std::function<void(std::string)> on_data;

  size_t send(const void* data, size_t size) {
    auto originSize = size;
    auto remainSize = size;
    while (remainSize != 0) {
      auto sendSize = client_->write((char*)data + (originSize - remainSize), remainSize);
      remainSize -= sendSize;
      if (sendSize == 0) {
        // todo:
        esp_rpc_LOGW("send size: %u", sendSize);
      }
    }

    size_t sendSize = originSize - remainSize;
    return sendSize;
  }

  void close() {
    client_->close(true);
  }

 private:
  AsyncClient* client_;

  friend class tcp_client;
};

struct tcp_server {
  explicit tcp_server(uint16_t port) : server_(port) {
    server_.onClient(
        [this](void*, AsyncClient* client) {
          esp_rpc_LOGD("new tcp_session: %p", client);
          auto tss = std::make_shared<tcp_session>(client);
          client->onDisconnect([tss](void*, AsyncClient*) {
            esp_rpc_LOGD("onDisconnect: %p", tss.get());
            if (tss->on_close) tss->on_close();
          });  // keep tss
          auto ws = std::weak_ptr<tcp_session>(tss);
          if (on_session) on_session(std::move(ws));
        },
        nullptr);
  }

  void start() {
    server_.begin();
  }

 public:
  std::function<void(std::weak_ptr<tcp_session>)> on_session;

 private:
  AsyncServer server_;
};

struct tcp_client : tcp_session {
  tcp_client() : tcp_session(new AsyncClient) {
    client_->onConnect([this](void*, AsyncClient*) {
      opening_ = false;
      if (on_open) on_open();
    });
    client_->onError([this](void*, AsyncClient*, err_t error) {
      if (!opening_) return;
      std::error_code ec;  // todo
      if (on_open_failed) on_open_failed(ec);
    });
  }

  void open(const std::string& ip, const std::string& port) {
    opening_ = true;
  }
  std::function<void()> on_open;
  std::function<void(std::error_code)> on_open_failed;

 private:
  bool opening_ = false;
};

void setTimeout(uint32_t ms, std::function<void()> cb);

}  // namespace esp_rpc
