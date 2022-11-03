#pragma once

#include <utility>

#include "RpcCore.hpp"
#include "detail/noncopyable.hpp"
#include "port_esp.hpp"

namespace esp_rpc {

class rpc_session : noncopyable {
 public:
  explicit rpc_session(std::weak_ptr<tcp_session> tcp_session) : tcp_session_(std::move(tcp_session)) {}

 public:
  void close() {
    auto ts = tcp_session_.lock();
    if (ts) {
      ts->close();
    }
  }

 public:
  std::function<void()> on_close;

 public:
  std::shared_ptr<RpcCore::Rpc> rpc;

 private:
  std::weak_ptr<tcp_session> tcp_session_;
};

}  // namespace esp_rpc
