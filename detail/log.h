/**
 * 统一控制调试信息
 * 为了保证输出顺序 都使用stdout而不是stderr
 *
 * 可配置项（默认都是未定义）
 * esp_rpc_LOG_NDEBUG               关闭esp_rpc_LOGD的输出
 * esp_rpc_LOG_SHOW_VERBOSE         显示esp_rpc_LOGV的输出
 * esp_rpc_LOG_DISABLE_COLOR        禁用颜色显示
 * esp_rpc_LOG_LINE_END_CRLF        默认是\n结尾 添加此宏将以\r\n结尾
 * esp_rpc_LOG_FOR_MCU              MCU项目可配置此宏 更适用于MCU环境
 * esp_rpc_LOG_NOT_EXIT_ON_FATAL    FATAL默认退出程序 添加此宏将不退出
 *
 * 其他配置项
 * esp_rpc_LOG_PRINTF_IMPL          定义输出实现（默认使用printf）
 * 并添加形如int esp_rpc_LOG_PRINTF_IMPL(const char *fmt, ...)的实现
 *
 * 在库中使用时
 * 1. 修改此文件中的`esp_rpc_LOG`以包含库名前缀（全部替换即可）
 * 2. 取消这行注释: #define esp_rpc_LOG_IN_LIB
 * 库中可配置项
 * esp_rpc_LOG_SHOW_DEBUG           开启esp_rpc_LOGD的输出
 *
 * 非库中使用时
 * esp_rpc_LOGD的输出在debug时打开 release时关闭（依据NDEBUG宏）
 */

#pragma once

// clang-format off

// 在库中使用时需取消注释
#define esp_rpc_LOG_IN_LIB

#ifdef __cplusplus
#include <cstring>
#include <cstdlib>
#else
#include <string.h>
#include <stdlib.h>
#endif

#ifdef  esp_rpc_LOG_LINE_END_CRLF
#define esp_rpc_LOG_LINE_END            "\r\n"
#else
#define esp_rpc_LOG_LINE_END            "\n"
#endif

#ifdef esp_rpc_LOG_NOT_EXIT_ON_FATAL
#define esp_rpc_LOG_EXIT_PROGRAM()
#else
#ifdef esp_rpc_LOG_FOR_MCU
#define esp_rpc_LOG_EXIT_PROGRAM()      do{ for(;;); } while(0)
#else
#define esp_rpc_LOG_EXIT_PROGRAM()      exit(EXIT_FAILURE)
#endif
#endif

#define esp_rpc_LOG_BASE_FILENAME       (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define esp_rpc_LOG_WITH_COLOR

#if defined(_WIN32) || defined(__ANDROID__) || defined(esp_rpc_LOG_FOR_MCU)
#undef esp_rpc_LOG_WITH_COLOR
#endif

#ifdef esp_rpc_LOG_DISABLE_COLOR
#undef esp_rpc_LOG_WITH_COLOR
#endif

#ifdef esp_rpc_LOG_WITH_COLOR
#define esp_rpc_LOG_COLOR_RED           "\033[31m"
#define esp_rpc_LOG_COLOR_GREEN         "\033[32m"
#define esp_rpc_LOG_COLOR_YELLOW        "\033[33m"
#define esp_rpc_LOG_COLOR_BLUE          "\033[34m"
#define esp_rpc_LOG_COLOR_CARMINE       "\033[35m"
#define esp_rpc_LOG_COLOR_CYAN          "\033[36m"
#define esp_rpc_LOG_COLOR_DEFAULT
#define esp_rpc_LOG_COLOR_END           "\033[m"
#else
#define esp_rpc_LOG_COLOR_RED
#define esp_rpc_LOG_COLOR_GREEN
#define esp_rpc_LOG_COLOR_YELLOW
#define esp_rpc_LOG_COLOR_BLUE
#define esp_rpc_LOG_COLOR_CARMINE
#define esp_rpc_LOG_COLOR_CYAN
#define esp_rpc_LOG_COLOR_DEFAULT
#define esp_rpc_LOG_COLOR_END
#endif

#define esp_rpc_LOG_END                 esp_rpc_LOG_COLOR_END esp_rpc_LOG_LINE_END

#if __ANDROID__
#include <android/log.h>
#define esp_rpc_LOG_PRINTF(...)         __android_log_print(ANDROID_L##OG_DEBUG, "esp_rpc_LOG", __VA_ARGS__)
#else
#define esp_rpc_LOG_PRINTF(...)         printf(__VA_ARGS__)
#endif

#ifndef esp_rpc_LOG_PRINTF_IMPL
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif
#define esp_rpc_LOG_PRINTF_IMPL(...)    esp_rpc_LOG_PRINTF(__VA_ARGS__) // NOLINT(bugprone-lambda-function-name)
#else
extern int esp_rpc_LOG_PRINTF_IMPL(const char *fmt, ...);
#endif

#define esp_rpc_LOG(fmt, ...)           do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_GREEN   "[*]: "             fmt esp_rpc_LOG_END, ##__VA_ARGS__); } while(0)
#define esp_rpc_LOGT(tag, fmt, ...)     do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_BLUE    "[" tag "]: "       fmt esp_rpc_LOG_END, ##__VA_ARGS__); } while(0)
#define esp_rpc_LOGI(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_YELLOW  "[I]: %s: "         fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#define esp_rpc_LOGW(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_CARMINE "[W]: %s: %s: %d: " fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); } while(0)                     // NOLINT(bugprone-lambda-function-name)
#define esp_rpc_LOGE(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_RED     "[E]: %s: %s: %d: " fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); } while(0)                     // NOLINT(bugprone-lambda-function-name)
#define esp_rpc_LOGF(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_CYAN    "[!]: %s: %s: %d: " fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, __func__, __LINE__, ##__VA_ARGS__); esp_rpc_LOG_EXIT_PROGRAM(); } while(0) // NOLINT(bugprone-lambda-function-name)

#if defined(esp_rpc_LOG_IN_LIB) && !defined(esp_rpc_LOG_SHOW_DEBUG) && !defined(esp_rpc_LOG_NDEBUG)
#define esp_rpc_LOG_NDEBUG
#endif

#if defined(NDEBUG) || defined(esp_rpc_LOG_NDEBUG)
#define esp_rpc_LOGD(fmt, ...)          ((void)0)
#else
#define esp_rpc_LOGD(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_DEFAULT "[D]: %s: "         fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#endif

#if defined(esp_rpc_LOG_SHOW_VERBOSE)
#define esp_rpc_LOGV(fmt, ...)          do{ esp_rpc_LOG_PRINTF_IMPL(esp_rpc_LOG_COLOR_DEFAULT "[V]: %s: "         fmt esp_rpc_LOG_END, esp_rpc_LOG_BASE_FILENAME, ##__VA_ARGS__); } while(0)
#else
#define esp_rpc_LOGV(fmt, ...)          ((void)0)
#endif
