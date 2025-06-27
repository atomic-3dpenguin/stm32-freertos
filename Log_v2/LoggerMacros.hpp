#pragma once
#include "Logger.hpp"

// ANSI colors
#define ANSI_COLOR_DEBUG "\x1B[36m"   // Cyan
#define ANSI_COLOR_INFO  "\x1B[32m"   // Green
#define ANSI_COLOR_WARN  "\x1B[33m"   // Yellow
#define ANSI_COLOR_ERROR "\x1B[31m"   // Red
#define ANSI_COLOR_RESET "\x1B[0m"

#define LOGMSG_DEBUG(fmt, ...) \
    common::logger::Logger::instance().log(common::logger::LogLevel::DEBUG, ANSI_COLOR_DEBUG "[DEBUG] " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define LOGMSG_INFO(fmt, ...) \
    common::logger::Logger::instance().log(common::logger::LogLevel::INFO, ANSI_COLOR_INFO "[INFO] " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define LOGMSG_WARN(fmt, ...) \
    common::logger::Logger::instance().log(common::logger::LogLevel::WARN, ANSI_COLOR_WARN "[WARN] " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

#define LOGMSG_ERROR(fmt, ...) \
    common::logger::Logger::instance().log(common::logger::LogLevel::ERROR, ANSI_COLOR_ERROR "[ERROR] " fmt ANSI_COLOR_RESET, ##__VA_ARGS__)
