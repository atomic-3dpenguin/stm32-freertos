// notified_task.hpp
#pragma once

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Base class for FreeRTOS tasks that use task notifications.
 *
 * This CRTP class provides a reusable structure for tasks that operate
 * on `ulTaskNotifyTake()` events. Derived classes must implement `onNotify()`.
 *
 * @tparam Derived The derived class implementing the onNotify() method
 */
template <typename Derived>
class NotifiedTask {
public:
    /**
     * @brief Construct the task and register it with FreeRTOS.
     *
     * @param name      Task name (for diagnostics)
     * @param stackSize Stack size in words (not bytes)
     * @param priority  FreeRTOS task priority
     */
    NotifiedTask(const char* name, uint16_t stackSize, UBaseType_t priority)
    {
        xTaskCreate(taskEntry, name, stackSize, static_cast<Derived*>(this), priority, &taskHandle);
    }

    /**
     * @brief Notify this task from an ISR.
     */
    void notifyFromISR()
    {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(taskHandle, &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

protected:
    /**
     * @brief Entry point for the derived class to handle a notification.
     * This method must be implemented by the derived class.
     */
    void runNotificationLoop()
    {
        while (true)
        {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            static_cast<Derived*>(this)->onNotify();
        }
    }

private:
    TaskHandle_t taskHandle = nullptr;

    static void taskEntry(void* param)
    {
        static_cast<Derived*>(param)->runNotificationLoop();
    }
};
