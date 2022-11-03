#include <utility>

#include "data_packer.hpp"
#include "port_esp.hpp"
#include "rpc_session.hpp"

namespace esp_rpc {

class rpc_server : noncopyable {
 public:
  explicit rpc_server(uint16_t port, uint32_t max_body_size = UINT32_MAX) : server_(port), data_packer_(max_body_size) {
    server_.on_session = [this](const std::weak_ptr<tcp_session>& ws) {
      auto rpc_session = std::make_shared<esp_rpc::rpc_session>(ws);
      auto rpc = RpcCore::Rpc::create();
      rpc_session->rpc = rpc;

      rpc->setTimer([](uint32_t ms, RpcCore::Rpc::TimeoutCb cb) {
        setTimeout(ms, std::move(cb));
      });

      rpc->getConn()->sendPackageImpl = [ws, this](const std::string& data) {
        auto ret = data_packer_.pack(data.data(), data.size(), [&](const void* data, size_t size) {
          return ws.lock()->send(data, size) == size;
        });

        if (!ret) {
          ws.lock()->close();
        }
      };

      auto session = ws.lock();
      session->on_close = [rpc_session] {
        if (rpc_session->on_close) {
          rpc_session->on_close();
        }
      };

      data_packer_.on_data = [rpc_session](std::string data) {
        rpc_session->rpc->getConn()->onRecvPackage(std::move(data));
      };

      session->on_data = [this](const std::string& data) {
        data_packer_.feed(data.data(), data.size());
      };

      if (on_session) {
        on_session(rpc_session);
      }
    };
  }

 public:
  void start() {
    server_.start();
  }

 public:
  std::function<void(std::weak_ptr<rpc_session>)> on_session;

 private:
  tcp_server server_;
  data_packer data_packer_;
};

}  // namespace esp_rpc
