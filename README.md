# esp_rpc

a Tiny RPC library for ESP32/ESP8266, and can be ported to any platform easily.

RPC based on [rpc_core](https://github.com/shuai132/rpc_core)

## Requirements

* C++11

* implement

```c++
namespace esp_rpc {
void dispatch(std::function<void()> runnable) {
    // dispatch runnable to main thread looper
    // because tcp callback may from interrupt or other thread 
}

void set_timeout(uint32_t ms, std::function<void()> cb) {
    // timeout implement
}
}  // namespace esp_rpc
```

## Usage

* RPC

```c++
  // server
  rpc_server server(8080);
  server.on_session = [](const std::weak_ptr<rpc_session>& rs) {
    auto session = rs.lock();
    session->on_close = [rs] {
    };
    session->rpc->subscribe("cmd", [](const std::string& data) -> std::string {
      return "world";
    });
  };
  server.start();
```

```c++
  // client
  rpc_client client;
  client.on_open = [&](const std::shared_ptr<rpc_core::rpc>& rpc) {
    rpc->cmd("cmd")
        ->msg(std::string("hello"))
        ->rsp([&](const std::string& data) {
        })
        ->call();
  };
  client.on_close = [&] {
  };
  client.open("localhost", 8080);
```

# Links

* for the platform with Operating System, there is a library based on
  asio: [asio_net](https://github.com/shuai132/asio_net)
