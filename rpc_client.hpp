#pragma once

#include <system_error>
#include <utility>

#include "detail/noncopyable.hpp"
#include "port_esp.hpp"
#include "rpc_core.hpp"
#include "rpc_session.hpp"

namespace esp_rpc {

class rpc_client : noncopyable {
 public:
  explicit rpc_client(uint32_t max_body_size = UINT32_MAX) : max_body_size_(max_body_size) {
    client_->on_open = [this]() {
      auto session = std::make_shared<rpc_session>(max_body_size_);
      session->init(client_);

      session->on_close = [this] {
        client_->on_data = nullptr;
        if (on_close) on_close();
      };

      if (on_open) on_open(session->rpc);
    };

    client_->on_open_failed = [this](const std::error_code& ec) {
      if (on_open_failed) on_open_failed(ec);
    };
  }

  void open(const std::string& ip, uint16_t port) {
    client_->open(ip, port);
  }

  void close() {
    client_->close();
  }

 public:
  std::function<void(std::shared_ptr<rpc_core::rpc>)> on_open;
  std::function<void()> on_close;
  std::function<void(std::error_code)> on_open_failed;

 private:
  uint32_t max_body_size_;
  std::shared_ptr<tcp_client> client_ = std::make_shared<tcp_client>();
};

}  // namespace esp_rpc
