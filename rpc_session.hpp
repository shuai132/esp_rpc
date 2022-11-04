#pragma once

#include <utility>

#include "RpcCore.hpp"
#include "detail/noncopyable.hpp"
#include "port_esp.hpp"

namespace esp_rpc {

class rpc_session : noncopyable, public std::enable_shared_from_this<rpc_session> {
 public:
  explicit rpc_session(uint32_t max_body_size) : data_packer_(max_body_size) {}

 public:
  void init(std::weak_ptr<tcp_session> ws) {
    tcp_session_ = std::move(ws);
    auto tcp_session = tcp_session_.lock();

    rpc = RpcCore::Rpc::create();

    rpc->setTimer([](uint32_t ms, RpcCore::Rpc::TimeoutCb cb) {
      setTimeout(ms, std::move(cb));
    });

    rpc->getConn()->sendPackageImpl = [this](const std::string& data) {
      auto tcp_session = tcp_session_.lock();
      if (!tcp_session) {
        esp_rpc_LOGW("tcp_session expired on sendPackage");
      }

      auto ret = data_packer_.pack(data.data(), data.size(), [&](const void* data, size_t size) {
        return tcp_session->send(data, size) == size;
      });

      if (!ret) {
        tcp_session->close();
      }
    };

    // bind rpc_session lifecycle to tcp_session
    tcp_session->on_close = [rpc_session = shared_from_this()] {
      if (rpc_session->on_close) {
        rpc_session->on_close();
      }
    };

    data_packer_.on_data = [this](std::string data) {
      rpc->getConn()->onRecvPackage(std::move(data));
    };

    tcp_session->on_data = [this](const std::string& data) {
      data_packer_.feed(data.data(), data.size());
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
  std::shared_ptr<RpcCore::Rpc> rpc;

 private:
  data_packer data_packer_;
  std::weak_ptr<tcp_session> tcp_session_;
};

}  // namespace esp_rpc
