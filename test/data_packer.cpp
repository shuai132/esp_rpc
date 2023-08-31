#include "data_packer.hpp"

#include <ctime>
#include <random>

#include "assert_def.h"

using namespace esp_rpc;

static void simpleUsage() {
  ESP_RPC_LOG();
  ESP_RPC_LOG("simpleUsage...");
  data_packer packer;
  std::string testData = "hello world";
  std::string packedData;

  packer.pack(testData.data(), testData.size(), [&](const void *data, size_t size) {
    packedData.insert(packedData.size(), (char *)data, size);
    return true;
  });
  ASSERT(packedData.size() == testData.size() + 4);

  std::string feedRecData;
  packer.on_data = [&](std::string data) {
    feedRecData = std::move(data);
  };
  packer.feed(packedData.data(), packedData.size());
  ASSERT(testData == feedRecData);
  ESP_RPC_LOG("packedData PASS");

  std::string packedData2 = packer.pack(testData);
  ASSERT(packedData2 == packedData);
  ESP_RPC_LOG("packedData2 PASS");

  ESP_RPC_LOG("feed again...");
  feedRecData.clear();
  ASSERT(testData != feedRecData);
  packer.feed(packedData.data(), packedData.size());
  ASSERT(testData == feedRecData);
}

static void testCommon() {
  ESP_RPC_LOG();
  ESP_RPC_LOG("testCommon...");
  ESP_RPC_LOG("generate big data...");
  bool pass = false;
  std::string TEST_PAYLOAD;
  int TestAddCount = 1000;
  for (int i = 0; i < TestAddCount; i++) {
    TEST_PAYLOAD += "helloworld";  // 10bytes
  }
  ESP_RPC_LOG("data generated, size:%zu", TEST_PAYLOAD.size());
  ASSERT(TEST_PAYLOAD.size() == TestAddCount * 10);

  data_packer packer;
  packer.on_data = [&](const std::string &data) {
    size_t size = data.size();
    ESP_RPC_LOG("get payload size:%zu", size);
    if (data == TEST_PAYLOAD) {
      pass = true;
    }
  };

  ESP_RPC_LOG("packing...");
  auto payload = packer.pack(TEST_PAYLOAD);
  const uint32_t payloadSize = payload.size();
  ESP_RPC_LOG("payloadSize:%u", payloadSize);
  ASSERT(payloadSize == TestAddCount * 10 + 4);

  ESP_RPC_LOG("******test normal******");
  packer.feed(payload.data(), payloadSize);
  ASSERT(pass);
  pass = false;

  ESP_RPC_LOG("******test random******");
  uint32_t sendLeft = payloadSize;
  std::default_random_engine generator(time(nullptr));  // NOLINT
  std::uniform_int_distribution<int> dis(1, 10);
  auto random = std::bind(dis, generator);  // NOLINT
  while (sendLeft > 0) {
    uint32_t randomSize = random();
    // ESP_RPC_LOG("random: %u,  %u", randomSize, sendLeft);
    size_t needSend = std::min(randomSize, sendLeft);
    packer.feed(payload.data() + (payloadSize - sendLeft), needSend);
    sendLeft -= needSend;
  }
  ASSERT(pass);
}

int main() {
  simpleUsage();
  testCommon();
  return 0;
}
