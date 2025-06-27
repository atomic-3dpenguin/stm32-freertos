#pragma once

#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <functional>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stm32g4xx_hal.h"  // or your specific device HAL

namespace common::logger {

constexpr size_t LOG_MESSAGE_MAX_SIZE = 128;
constexpr size_t LOG_QUEUE_LENGTH     = 16;

enum class LogLevel : uint8_t {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

struct LogMessage {
    LogLevel level;
    char message[LOG_MESSAGE_MAX_SIZE];
};

using LogOutputFunc = std::function<void(const LogMessage&)>;

class Logger {
public:
    static Logger& instance() {
        static Logger self;
        return self;
    }

    void start() {
        xTaskCreate(taskFunc, "LoggerTask", 512, this, tskIDLE_PRIORITY + 1, nullptr);
    }

    void setOutput(LogOutputFunc func) {
        outputFunc_ = func;
    }

    void log(LogLevel level, const char* fmt, ...) {
        if (!queue_) return;

        LogMessage msg;
        msg.level = level;

        char timestamp[20];
        formatTimestampRTC(timestamp, sizeof(timestamp));

        char userContent[LOG_MESSAGE_MAX_SIZE - 32];
        va_list args;
        va_start(args, fmt);
        vsnprintf(userContent, sizeof(userContent), fmt, args);
        va_end(args);

        snprintf(msg.message, sizeof(msg.message), "[%s] %s", timestamp, userContent);

        if (isInISR()) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendToBackFromISR(queue_, &msg, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        } else {
            xQueueSendToBack(queue_, &msg, 0);
        }
    }

private:
    Logger() {
        queue_ = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogMessage));
    }

    static void taskFunc(void* param) {
        auto* self = static_cast<Logger*>(param);
        LogMessage msg;

        while (true) {
            if (xQueueReceive(self->queue_, &msg, portMAX_DELAY) == pdTRUE && self->outputFunc_) {
                self->outputFunc_(msg);
            }
        }
    }

    static bool isInISR() {
        return __get_IPSR() != 0;
    }

    static void formatTimestampRTC(char* buffer, size_t len) {
        RTC_TimeTypeDef sTime = {};
        RTC_DateTypeDef sDate = {};
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  // required to unlock RTC time read

        uint32_t ticks = xTaskGetTickCount();
        uint32_t ms = (1000UL * ticks / configTICK_RATE_HZ) % 1000;

        snprintf(buffer, len, "%02u:%02u:%02u.%03lu",
                 sTime.Hours, sTime.Minutes, sTime.Seconds, ms);
    }

    QueueHandle_t queue_ = nullptr;
    LogOutputFunc outputFunc_ = nullptr;
};

// Declare globally visible RTC handle
extern RTC_HandleTypeDef hrtc;

} // namespace common::logger
