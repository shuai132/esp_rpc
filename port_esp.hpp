#pragma once

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

#include <cstdint>
#include <string>

namespace esp_rpc {

class tcp_session {
 public:
  explicit tcp_session(AsyncClient* client) : client_(client) {
    esp_rpc_LOGD("tcp_session new: %p, client: %p", this, client);
    client->onData([this](void*, AsyncClient*, void* data, size_t len) {
      if (on_data) on_data((uint8_t*)data, len);
    });
  }

  ~tcp_session() {
    esp_rpc_LOGD("~tcp_session: %p", this);
  }

  std::function<void()> on_close;

  std::function<void(uint8_t* data, size_t size)> on_data;

  size_t send(const void* data, size_t size) {
    auto originSize = size;
    auto remainSize = size;
    while (remainSize != 0) {
      auto sendSize = client_->write((char*)data + (originSize - remainSize), remainSize);
      remainSize -= sendSize;
      if (sendSize == 0) {
        esp_rpc_LOGW("sendSize == 0");
        delay(10);
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

class tcp_server {
 public:
  explicit tcp_server(uint16_t port) : server_(port) {
    server_.onClient(
        [this](void*, AsyncClient* client) {
          esp_rpc_LOGD("onClient: %p", client);
          auto tss = std::make_shared<tcp_session>(client);
          // bind tcp_session lifecycle to AsyncClient
          client->onDisconnect([tss](void*, AsyncClient* client) {
            esp_rpc_LOGD("onDisconnect: %p", client);
            if (tss->on_close) tss->on_close();
          });
          if (on_session) on_session(tss);
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

class tcp_client : public tcp_session {
 public:
  tcp_client() : tcp_session(new AsyncClient) {
    client_->onConnect([this](void*, AsyncClient*) {
      opening_ = false;
      if (on_open) on_open();
    });

    client_->onError([this](void*, AsyncClient*, err_t error) {
      esp_rpc_LOGE("onError: %ld", error);
      opening_ = false;
      if (!opening_) return;
      if (on_open_failed) on_open_failed(std::make_error_code(static_cast<std::errc>(error)));
    });

    client_->onDisconnect([this](void*, AsyncClient* client) {
      esp_rpc_LOGD("onDisconnect: %p", client);
      opening_ = false;
      if (on_close) on_close();
    });
  }

  ~tcp_client() {
    delete client_;
  }

  void open(const std::string& ip, uint16_t port) {
    if (opening_) return;
    opening_ = true;
    client_->connect(ip.c_str(), port);
  }
  std::function<void()> on_open;
  std::function<void(std::error_code)> on_open_failed;

 private:
  bool opening_ = false;
};

void setTimeout(uint32_t ms, std::function<void()> cb);

}  // namespace esp_rpc
