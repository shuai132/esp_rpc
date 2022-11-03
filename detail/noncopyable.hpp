#pragma once

namespace esp_rpc {

class noncopyable {
 protected:
  noncopyable() = default;
  ~noncopyable() = default;

 public:
  noncopyable(const noncopyable&) = delete;
  const noncopyable& operator=(const noncopyable&) = delete;
};

}  // namespace esp_rpc
