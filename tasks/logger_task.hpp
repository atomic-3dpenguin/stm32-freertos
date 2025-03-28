// logger_task.hpp
#pragma once

#include "notified_task.hpp"
#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f4xx_hal.h"
#include <cstdarg>
#include <cstdio>
#include <array>
#include <cstring>

/**
 * @brief Task responsible for logging messages to UART with different log levels.
 */
class LoggerTask : public NotifiedTask<LoggerTask> {
public:
    enum Level { LOG_INFO, LOG_WARN, LOG_ERROR };

    LoggerTask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
        : NotifiedTask<LoggerTask>(name, stackSize, priority), huart(uart)
    {
        logQueue = xQueueCreate(16, sizeof(Buffer));
    }

    /**
     * @brief Logs a formatted message with the specified log level.
     *
     * @param level Logging severity level
     * @param fmt printf-style format string
     * @param ... Variadic arguments for formatting
     */
    void log(Level level, const char* fmt, ...) {
        Buffer buf{};
        const char* prefix = level == LOG_INFO ? "[INFO] " : level == LOG_WARN ? "[WARN] " : "[ERROR] ";
        std::snprintf(buf.data, buf.size, "%s", prefix);

        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buf.data + std::strlen(prefix), buf.size - std::strlen(prefix), fmt, args);
        va_end(args);

        xQueueSend(logQueue, &buf, 0);
        this->notifyFromISR();
    }

    /**
     * @brief Called when the task is notified to transmit pending log messages.
     */
    void onNotify() {
        Buffer buf{};
        if (xQueueReceive(logQueue, &buf, 0) == pdTRUE) {
            HAL_UART_Transmit(huart, reinterpret_cast<uint8_t*>(buf.data), std::strlen(buf.data), HAL_MAX_DELAY);
        }
    }

private:
    UART_HandleTypeDef* huart;

    struct Buffer {
        static constexpr std::size_t size = 128;
        char data[size]{};
    };

    QueueHandle_t logQueue;
};
