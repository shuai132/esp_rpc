# esp_rpc

a Tiny RPC library for ESP32/ESP8266, and can be port to any platform easily.

RPC based on [RpcCore](https://github.com/shuai132/RpcCore)

## Requirements

* C++11

## Usage

* RPC

```c++
  // server
  rpc_server server(8080);
  server.on_session = [](const std::weak_ptr<rpc_session>& rs) {
    auto session = rs.lock();
    session->on_close = [rs] {
    };
    session->rpc->subscribe("cmd", [](const RpcCore::String& data) -> RpcCore::String {
      return "world";
    });
  };
  server.start();
```

```c++
  // client
  rpc_client client;
  client.on_open = [&](const std::shared_ptr<RpcCore::Rpc>& rpc) {
    rpc->cmd("cmd")
        ->msg(RpcCore::String("hello"))
        ->rsp([&](const RpcCore::String& data) {
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
