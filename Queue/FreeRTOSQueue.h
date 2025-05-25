#ifndef FREERTOS_QUEUE_H_
#define FREERTOS_QUEUE_H_

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "task.h"
#include "osal/queue/OsalQueue.h"

#include <chrono>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstring>

using namespace Osal::queue;

namespace OS::FreeRTOS {

    static constexpr auto WAIT_FOREVER = std::chrono::milliseconds(portMAX_DELAY * portTICK_PERIOD_MS);

    template <typename M, size_t MAX = 1>
    class alignas(alignof(M)) FreeRTOSQueue : public OsalQueue<FreeRTOSQueue<M, MAX>, M, MAX> {
      public:
        using MessageObject = M;
        static constexpr size_t MAXIMUM_QUEUE_MESSAGES = MAX;

        FreeRTOSQueue() {
#ifdef DEBUG
            std::memset(&Storage, 0xA5, sizeof(Storage));
#endif
            QueueHandle = xQueueCreateStatic(MAXIMUM_QUEUE_MESSAGES,
                                             sizeof(MessageObject),
                                             reinterpret_cast<uint8_t*>(&Storage),
                                             &xStaticQueue);

            configASSERT(QueueHandle != nullptr);
            assertAlignment();
        }

        [[nodiscard]] QUEUE_STATUS send(const MessageObject& message,
                                        std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
                                        BaseType_t* xHigherPriorityTaskWoken = nullptr) {
            if (xPortIsInsideInterrupt() == pdTRUE) {
                BaseType_t localHPW = pdFALSE;
                BaseType_t* hpw = (xHigherPriorityTaskWoken != nullptr) ? xHigherPriorityTaskWoken : &localHPW;

                auto result = xQueueSendFromISR(QueueHandle, &message, hpw);
                if (!xHigherPriorityTaskWoken) {
                    portYIELD_FROM_ISR(localHPW);
                }
                return (result == pdTRUE) ? QUEUE_STATUS::QUEUE_SUCCESS : QUEUE_STATUS::QUEUE_ERR_QUEUE_FULL;
            } else {
                return (xQueueSend(QueueHandle, &message, chrono_ms_to_rtos_ticks(timeout)) == pdTRUE)
                           ? QUEUE_STATUS::QUEUE_SUCCESS
                           : QUEUE_STATUS::QUEUE_ERR_QUEUE_FULL;
            }
        }

        [[nodiscard]] std::optional<MessageObject> receive(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
            MessageObject message;
            BaseType_t result = pdFALSE, xHigherPriorityTaskWoken = pdFALSE;

            if (xPortIsInsideInterrupt() == pdTRUE) {
                result = xQueueReceiveFromISR(QueueHandle, &message, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            } else {
                result = xQueueReceive(QueueHandle, &message, chrono_ms_to_rtos_ticks(timeout));
            }

            return (result == pdPASS) ? std::optional<MessageObject>{message} : std::nullopt;
        }

        [[nodiscard]] bool hasMessage() const {
            return (uxQueueMessagesWaiting(QueueHandle) > 0);
        }

        [[nodiscard]] QueueHandle_t raw() const { return QueueHandle; }

        [[nodiscard]] StaticQueue_t* getStaticQueue() { return &xStaticQueue; }

        [[nodiscard]] void* getStorageBuffer() { return &Storage; }

        void assertAlignment() const {
            configASSERT(reinterpret_cast<uintptr_t>(&Storage) % alignof(MessageObject) == 0);
            configASSERT(reinterpret_cast<uintptr_t>(this) % alignof(MessageObject) == 0);
        }

      private:
        TickType_t chrono_ms_to_rtos_ticks(const std::chrono::milliseconds& ms) const {
            return static_cast<TickType_t>(ms.count() / portTICK_PERIOD_MS);
        }

        // Aligned memory buffer
        using AlignedStorage = std::aligned_storage_t<sizeof(MessageObject) * MAXIMUM_QUEUE_MESSAGES,
                                                      alignof(MessageObject)>;

        AlignedStorage Storage;

        StaticQueue_t xStaticQueue;
        QueueHandle_t QueueHandle;

        static_assert(alignof(FreeRTOSQueue) >= alignof(MessageObject),
                      "FreeRTOSQueue object itself must be aligned for MessageObject");
        static_assert(sizeof(Storage) >= sizeof(MessageObject) * MAXIMUM_QUEUE_MESSAGES,
                      "Queue storage is too small for message pool");
    };

}  // namespace OS::FreeRTOS

#endif  // FREERTOS_QUEUE_H_
