#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "port_common.h"
#include "port_error.hpp"

namespace esp_rpc {

class tcp_session {
 public:
  explicit tcp_session(AsyncClient* client) : client_(client) {
    esp_rpc_LOGD("tcp_session new: %p, client: %p", this, client);
    client->onData([this](void*, AsyncClient*, void* data, size_t size) {
      dispatch([this, data = std::string((char*)data, size)] {
        if (on_data) on_data((uint8_t*)data.data(), data.size());
      });
    });
  }

  ~tcp_session() {
    esp_rpc_LOGD("~tcp_session: %p", this);
  }

  size_t send(const void* data, size_t size) {
    auto originSize = size;
    auto remainSize = size;
    int retryCount = 0;
    while (client_->connected() && remainSize != 0) {
      auto sendSize = client_->write((char*)data + (originSize - remainSize), remainSize);
      remainSize -= sendSize;
      if (sendSize == 0) {
        if (++retryCount <= 3) {
          esp_rpc_LOGW("sendSize == 0, remainSize: %zu", remainSize);
          delay(100);
        } else {
          break;
        }
      } else {
        retryCount = 0;
      }
    }

    size_t sendSize = originSize - remainSize;
    return sendSize;
  }

  void close() {
    client_->close(true);
  }

 public:
  std::function<void()> on_close;
  std::function<void(uint8_t* data, size_t size)> on_data;

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
          client->onDisconnect([tss](void*, AsyncClient* client) mutable {
            esp_rpc_LOGD("onDisconnect: %p", client);
            dispatch([tss = std::move(tss)] {
              if (tss->on_close) tss->on_close();
            });
          });
          dispatch([this, tss = std::move(tss)] {
            if (on_session) on_session(tss);
          });
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
      dispatch([this] {
        if (on_open) on_open();
      });
    });

    client_->onError([this](void*, AsyncClient*, err_t error) {
      esp_rpc_LOGE("onError: %ld, %s", error, client_->errorToString(error));
      if (opening_) {
        dispatch([this, error] {
          if (on_open_failed) on_open_failed(ErrorType(error));
        });
      }
      opening_ = false;
    });

    client_->onDisconnect([this](void*, AsyncClient* client) {
      esp_rpc_LOGD("onDisconnect: %p", client);
      opening_ = false;
      dispatch([this] {
        if (on_close) on_close();
      });
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

 public:
  std::function<void()> on_open;
  std::function<void(std::error_code)> on_open_failed;

 private:
  bool opening_ = false;
};

}  // namespace esp_rpc
