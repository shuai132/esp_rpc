#pragma once

#include <system_error>
#include <utility>

#include "RpcCore.hpp"
#include "data_packer.hpp"
#include "detail/noncopyable.hpp"
#include "port_esp.hpp"
#include "rpc_session.hpp"

namespace esp_rpc {

class rpc_client : noncopyable {
 public:
  explicit rpc_client(uint32_t max_body_size = UINT32_MAX) : data_packer_(max_body_size) {
    client_.on_open = [this]() {
      auto rpc = RpcCore::Rpc::create();

      rpc->setTimer([](uint32_t ms, RpcCore::Rpc::TimeoutCb cb) {
        setTimeout(ms, std::move(cb));
      });

      rpc->getConn()->sendPackageImpl = [this](const std::string& data) {
        auto ret = data_packer_.pack(data.data(), data.size(), [&](const void* data, size_t size) {
          return client_.send(data, size) == size;
        });

        if (!ret) {
          client_.close();
        }
      };

      data_packer_.on_data = [rpc](std::string data) {
        rpc->getConn()->onRecvPackage(std::move(data));
      };
      client_.on_data = [this](const std::string& data) {
        data_packer_.feed(data.data(), data.size());
      };

      on_open(rpc);
    };

    client_.on_close = [this] {
      client_.on_data = nullptr;
      on_close();
    };

    client_.on_open_failed = [this](const std::error_code& ec) {
      if (on_open_failed) on_open_failed(ec);
    };
  }

  void open(const std::string& ip, const std::string& port) {
    client_.open(ip, port);
  }

  void close() {
    client_.close();
  }

 public:
  std::function<void(std::shared_ptr<RpcCore::Rpc>)> on_open;
  std::function<void()> on_close;
  std::function<void(std::error_code)> on_open_failed;

 private:
  tcp_client client_;
  data_packer data_packer_;
};

}  // namespace esp_rpc
