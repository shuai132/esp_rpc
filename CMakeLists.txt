cmake_minimum_required(VERSION 3.1)

project(esp_rpc CXX)

option(ESP_RPC_BUILD_TEST "" OFF)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ESP_RPC_BUILD_TEST ON)
endif ()

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-Wall)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)
