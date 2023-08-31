/**
 * 统一控制调试信息
 * 为了保证输出顺序 都使用stdout而不是stderr
 *
 * 可配置项（默认都是未定义）
 * ESP_RPC_LOG_NDEBUG               关闭ESP_RPC_LOGD的输出
 * ESP_RPC_LOG_SHOW_VERBOSE         显示ESP_RPC_LOGV的输出
 * ESP_RPC_LOG_DISABLE_COLOR        禁用颜色显示
 * ESP_RPC_LOG_LINE_END_CRLF        默认是\n结尾 添加此宏将以\r\n结尾
 * ESP_RPC_LOG_FOR_MCU              MCU项目可配置此宏 更适用于MCU环境
 * ESP_RPC_LOG_NOT_EXIT_ON_FATAL    FATAL默认退出程序 添加此宏将不退出
 *
 * 其他配置项
 * ESP_RPC_LOG_PRINTF_IMPL          定义输出实现（默认使用printf）
 * 并添加形如int ESP_RPC_LOG_PRINTF_IMPL(const char *fmt, ...)的实现
 *
 * 在库中使用时
 * 1. 修改此文件中的`ESP_RPC_LOG`以包含库名前缀（全部替换即可）
 * 2. 取消这行注释: #define ESP_RPC_LOG_IN_LIB
 * 库中可配置项
 * ESP_RPC_LOG_SHOW_DEBUG           开启ESP_RPC_LOGD的输出
 *
 * 非库中使用时
 * ESP_RPC_LOGD的输出在debug时打开 release时关闭（依据NDEBUG宏）
 */

#pragma once

// clang-format off

// 在库中使用时需取消注释
#define ESP_RPC_LOG_IN_LIB

#ifdef __cplusplus
#include <cstring>
#include <cstdlib>
#else
#include <string.h>
#include <stdlib.h>
#endif

#ifdef  ESP_RPC_LOG_LINE_END_CRLF
#define ESP_RPC_LOG_LINE_END            "\r\n"
#else
#define ESP_RPC_LOG_LINE_END            "\n"
#endif

#ifdef ESP_RPC_LOG_NOT_EXIT_ON_FATAL
#define ESP_RPC_LOG_EXIT_PROGRAM()
#else
#ifdef ESP_RPC_LOG_FOR_MCU
#define ESP_RPC_LOG_EXIT_PROGRAM()      do{ for(;;); } while(0)
#else
#define ESP_RPC_LOG_EXIT_PROGRAM()      exit(EXIT_FAILURE)
#endif
#endif

#define ESP_RPC_LOG_BASE_FILENAME       (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define ESP_RPC_LOG_WITH_COLOR

#if defined(_WIN32) || defined(__ANDROID__) || defined(ESP_RPC_LOG_FOR_MCU)
#undef ESP_RPC_LOG_WITH_COLOR
#endif

#ifdef ESP_RPC_LOG_DISABLE_COLOR
#undef ESP_RPC_LOG_WITH_COLOR
#endif

#ifdef ESP_RPC_LOG_WITH_COLOR
#define ESP_RPC_LOG_COLOR_RED           "\033[31m"
#define ESP_RPC_LOG_COLOR_GREEN         "\033[32m"
#define ESP_RPC_LOG_COLOR_YELLOW        "\033[33m"
#define ESP_RPC_LOG_COLOR_BLUE          "\033[34m"
#define ESP_RPC_LOG_COLOR_CARMINE       "\033[35m"
#define ESP_RPC_LOG_COLOR_CYAN          "\033[36m"
#define ESP_RPC_LOG_COLOR_DEFAULT
#define ESP_RPC_LOG_COLOR_END           "\033[m"
#else
#define ESP_RPC_LOG_COLOR_RED
#define ESP_RPC_LOG_COLOR_GREEN
#define ESP_RPC_LOG_COLOR_YELLOW
#define ESP_RPC_LOG_COLOR_BLUE
#define ESP_RPC_LOG_COLOR_CARMINE
#define ESP_RPC_LOG_COLOR_CYAN
#define ESP_RPC_LOG_COLOR_DEFAULT
#define ESP_RPC_LOG_COLOR_END
#endif

#define ESP_RPC_LOG_END                 ESP_RPC_LOG_COLOR_END ESP_RPC_LOG_LINE_END

#if __ANDROID__
#include <android/log.h>
#define ESP_RPC_LOG_PRINTF(...)         __android_log_print(ANDROID_L##OG_DEBUG, "ESP_RPC_LOG", __VA_ARGS__)
#else
#define ESP_RPC_LOG_PRINTF(...)         printf(__VA_ARGS__)
#endif

#ifndef ESP_RPC_LOG_PRINTF_IMPL
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif
#define ESP_RPC_LOG_PRINTF_IMPL(...)    ESP_RPC_LOG_PRINTF(__VA_ARGS__) // NOLINT(bugprone-lambda-function-name)
#else
extern int ESP_RPC_LOG_PRINTF_IMPL(const char *fmt, ...);
#endif

#define ESP_RPC_LOG(fmt, ...)           do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_GREEN   "[*]: "             fmt ESP_RPC_LOG_END, ##__VA_ARGS__); } while(0)
#define ESP_RPC_LOGT(tag, fmt, ...)     do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_BLUE    "[" tag "]: "       fmt ESP_RPC_LOG_END, ##__VA_ARGS__); } while(0)
#define ESP_RPC_LOGI(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_YELLOW  "[I]: %s: "         fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#define ESP_RPC_LOGW(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_CARMINE "[W]: %s: %s: %d: " fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); } while(0)                     // NOLINT(bugprone-lambda-function-name)
#define ESP_RPC_LOGE(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_RED     "[E]: %s: %s: %d: " fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); } while(0)                     // NOLINT(bugprone-lambda-function-name)
#define ESP_RPC_LOGF(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_CYAN    "[!]: %s: %s: %d: " fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); ESP_RPC_LOG_EXIT_PROGRAM(); } while(0) // NOLINT(bugprone-lambda-function-name)

#if defined(ESP_RPC_LOG_IN_LIB) && !defined(ESP_RPC_LOG_SHOW_DEBUG) && !defined(ESP_RPC_LOG_NDEBUG)
#define ESP_RPC_LOG_NDEBUG
#endif

#if defined(NDEBUG) || defined(ESP_RPC_LOG_NDEBUG)
#define ESP_RPC_LOGD(fmt, ...)          ((void)0)
#else
#define ESP_RPC_LOGD(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_DEFAULT "[D]: %s: "         fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#endif

#if defined(ESP_RPC_LOG_SHOW_VERBOSE)
#define ESP_RPC_LOGV(fmt, ...)          do{ ESP_RPC_LOG_PRINTF_IMPL(ESP_RPC_LOG_COLOR_DEFAULT "[V]: %s: "         fmt ESP_RPC_LOG_END, ESP_RPC_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#else
#define ESP_RPC_LOGV(fmt, ...)          ((void)0)
#endif
