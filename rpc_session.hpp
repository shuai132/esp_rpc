#pragma once

#include <utility>

#include "detail/noncopyable.hpp"
#include "port_esp.hpp"
#include "rpc_core.hpp"

namespace esp_rpc {

class rpc_session : noncopyable, public std::enable_shared_from_this<rpc_session> {
 public:
  explicit rpc_session(uint32_t max_body_size) : stream_connection_(std::make_shared<rpc_core::stream_connection>(max_body_size)) {}

 public:
  void init(std::weak_ptr<tcp_session> ws) {
    tcp_session_ = std::move(ws);
    auto tcp_session = tcp_session_.lock();

    rpc = rpc_core::rpc::create(stream_connection_);

    rpc->set_timer([](uint32_t ms, rpc_core::rpc::timeout_cb cb) {
      set_timeout(ms, std::move(cb));
    });

    stream_connection_->send_bytes_impl = [this](const std::string& data) {
      auto tcp_session = tcp_session_.lock();
      if (!tcp_session) {
        ESP_RPC_LOGW("tcp_session expired on sendPackage");
      }

      auto ret = tcp_session->send(data.data(), data.size()) == data.size();
      if (!ret) {
        tcp_session->close();
      }
    };

    rpc->set_ready(true);

    // bind rpc_session lifecycle to tcp_session and end with on_close
    tcp_session->on_close = [this, rpc_session = shared_from_this()]() mutable {
      rpc->set_ready(false);

      if (!rpc_session) return;
      if (rpc_session->on_close) {
        rpc_session->on_close();
      }
      rpc_session = nullptr;
    };

    tcp_session->on_data = [this](uint8_t* data, size_t size) {
      stream_connection_->on_recv_bytes(data, size);
    };
  }

  void close() {
    auto ts = tcp_session_.lock();
    if (ts) {
      ts->close();
    }
  }

 public:
  std::function<void()> on_close;

 public:
  std::shared_ptr<rpc_core::rpc> rpc;

 private:
  std::shared_ptr<rpc_core::stream_connection> stream_connection_;
  std::weak_ptr<tcp_session> tcp_session_;
};

}  // namespace esp_rpc
