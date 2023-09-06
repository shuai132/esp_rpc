#include <utility>

#include "rpc_session.hpp"

namespace esp_rpc {

class rpc_server : noncopyable {
 public:
  explicit rpc_server(uint16_t port, uint32_t max_body_size = UINT32_MAX) : server_(port), max_body_size_(max_body_size) {
    server_.on_session = [this](std::weak_ptr<tcp_session> ws) {
      auto session = std::make_shared<rpc_session>(max_body_size_);
      session->init(std::move(ws));
      if (on_session) on_session(session);
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
  uint32_t max_body_size_;
};

}  // namespace esp_rpc
