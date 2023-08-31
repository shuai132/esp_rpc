#pragma once

#include <cstdint>
#include <functional>

// platform independent
namespace esp_rpc {
void dispatch(std::function<void()> runnable);
void set_timeout(uint32_t ms, std::function<void()> cb);
}  // namespace esp_rpc
