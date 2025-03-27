// logger_task.hpp
#pragma once

#include "uart_dma_task.hpp"
#include <cstdarg>

class LoggerTask : public UARTDMATask {
public:
    enum LogLevel {
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_COUNT
    };

    LoggerTask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart);

    void log(LogLevel level, const char* format, ...);

private:
    static constexpr const char* levelPrefixes[LOG_COUNT] = {
        "[INFO] ", "[WARN] ", "[ERROR] "
    };
};
