#ifdef ESP8266

#pragma once

#include <array>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "ESPAsyncUDP.h"
#include "WiFiUdp.h"
#include "detail/log.h"
#include "port_common.h"
#include "src/Type.hpp"

namespace esp_rpc {

/**
 * message format:
 * prefix + '\n' + name + '\n' + message
 * i.e: "discovery\nname\nmessage"
 */
namespace server_discovery {

const char* addr_default = "239.255.0.1";
const uint16_t port_default = 30001;

using timer_impl_t = std::function<void(uint32_t ms, std::function<void()>)>;

class receiver {
  using service_found_handle_t = std::function<void(std::string name, std::string message)>;

 public:
  explicit receiver(service_found_handle_t handle, const std::string& addr = addr_default, uint16_t port = port_default)
      : port_(port), service_found_handle_(std::move(handle)) {
    ipAddress_.fromString(addr.c_str());
    try_init();
  }

 private:
  void try_init() {
    bool ret = udp_.listenMulticast(ipAddress_, port_);
    if (ret) {
      do_receive();
    } else {
      esp_rpc_LOGE("receive: init err");
      auto alive = std::weak_ptr<void>(isAlive_);
      timer_(1000, [this, RpcCore_MOVE_LAMBDA(alive)] {
        if (alive.expired()) return;
        try_init();
      });
    }
  }

  void do_receive() {
    udp_.onPacket([this](AsyncUDPPacket packet) {
      std::vector<std::string> msgs;
      {
        msgs.reserve(3);
        auto begin = packet.data();
        auto end = packet.data() + packet.length();
        decltype(begin) it;
        int count = 0;
        while ((it = std::find(begin, end, '\n')) != end) {
          msgs.emplace_back(begin, it);
          begin = it + 1;
          if (++count > 2) break;
        }
        msgs.emplace_back(begin, it);
      }
      if (msgs.size() == 3 && msgs[0] == "discovery") {
        dispatch([this, msgs = std::move(msgs)]() mutable {
          service_found_handle_(std::move(msgs[1]), std::move(msgs[2]));
        });
      }
    });
  }

  AsyncUDP udp_;
  IPAddress ipAddress_;
  uint16_t port_;
  service_found_handle_t service_found_handle_;
  timer_impl_t timer_;
  std::shared_ptr<void> isAlive_ = std::make_shared<uint8_t>();
};

class sender {
 public:
  sender(timer_impl_t timerImpl, const std::string& service_name, const std::string& message, uint send_period_sec = 1,
         const char* addr = addr_default, uint16_t port = port_default)
      : timer_(std::move(timerImpl)), port_(port), send_period_ms_(send_period_sec * 1000), message_("discovery\n" + service_name + '\n' + message) {
    ipAddress_.fromString(addr);
    do_send();
  }

 private:
  void do_send() {
    IPAddress ipAddress = WiFi.softAPIP();
    if (!ipAddress.isSet()) {
      ipAddress = WiFi.localIP();
    }
    udp_.beginPacketMulticast(ipAddress_, port_, ipAddress);
    udp_.write(message_.c_str());
    auto ret = udp_.endPacket();
    if (!ret) {
      esp_rpc_LOGE("do_send: addr: %s, ret: %d", ipAddress.toString().c_str(), ret);
    }
    do_send_next();
  }

  void do_send_next() {
    auto alive = std::weak_ptr<void>(isAlive_);
    timer_(send_period_ms_, [this, RpcCore_MOVE_LAMBDA(alive)] {
      if (alive.expired()) return;
      do_send();
    });
  }

 private:
  WiFiUDP udp_;
  timer_impl_t timer_;
  IPAddress ipAddress_;
  uint8_t port_;

  uint send_period_ms_;
  std::string message_;
  std::shared_ptr<void> isAlive_ = std::make_shared<uint8_t>();
};

}  // namespace server_discovery
}  // namespace esp_rpc

#endif
