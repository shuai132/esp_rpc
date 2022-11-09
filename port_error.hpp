#pragma once

#include <string>
#include <system_error>

#ifdef ESP8266
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include "AsyncTCP.h"
#else
#error "platform not support"
#endif

namespace esp_rpc {

enum class ErrorType : err_t {};

class ErrorCategory : public std::error_category {
 public:
  static ErrorCategory& instance() {
    static ErrorCategory instance;
    return instance;
  }

 public:
  ErrorCategory() = default;
  const char* name() const noexcept override {
    return "lwip error";
  }
  std::string message(int ev) const override {
    return ((AsyncClient*)(nullptr))->errorToString((err_t)ev);
  }
};

std::error_code make_error_code(ErrorType e) {
  return {static_cast<int>(e), ErrorCategory::instance()};
}

}  // namespace esp_rpc

namespace std {
template <>
struct is_error_code_enum<esp_rpc::ErrorType> : public true_type {};
}  // namespace std
