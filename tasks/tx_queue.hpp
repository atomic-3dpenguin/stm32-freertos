// tx_queue.hpp
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include <cstring>
#include <array>

/**
 * @brief Manages a simple FreeRTOS queue for sending UART data.
 */
class TxQueueManager {
public:
    static constexpr std::size_t MaxMsgSize = 128;
    static constexpr std::size_t QueueLength = 16;

    TxQueueManager() {
        queue = xQueueCreate(QueueLength, MaxMsgSize);
    }

    bool enqueue(const char* msg) {
        if (msg && std::strlen(msg) < MaxMsgSize) {
            return xQueueSend(queue, msg, 0) == pdTRUE;
        }
        return false;
    }

    bool dequeue(char* out) {
        return xQueueReceive(queue, out, 0) == pdTRUE;
    }

    bool hasMessages() const {
        return uxQueueMessagesWaiting(queue) > 0;
    }

private:
    QueueHandle_t queue;
};
