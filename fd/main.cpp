// =============================
// File: main.cpp
// =============================
#include "can_fd_driver.hpp"
#include "can_fd_hal_if.hpp"
#include "FreeRTOS.h"
#include "task.h"

// Declare the task handle (defined in callbacks file)
extern TaskHandle_t CanTaskHandle;

// Forward: the CAN-task that dispatches events
void CanTask(void* /*pv*/) {
    can::EventFlags v;
    for (;;) {
        if (xTaskNotifyWait(0, ULONG_MAX, &v, portMAX_DELAY) == pdTRUE) {
            uint8_t driverId = static_cast<uint8_t>(v >> 24);
            auto    events   = v & 0x00FFFFFF;
            if (driverId != 1) continue;  // only our driver

            if (events & Event_RxPending) {
                // HAL_FDCAN_GetRxMessage, etc.
            }
            if (events & Event_TxComplete) {
                // post-TX housekeeping
            }
            if (events & Event_Error) {
                // error recovery
            }
        }
    }
}

int main() {
    // 1) Create the CAN task
    xTaskCreate(
      CanTask, "CAN", configMINIMAL_STACK_SIZE+128,
      nullptr, tskIDLE_PRIORITY+2, &CanTaskHandle
    );

    // 2) Init HAL & driver
    HAL_Init();
    MyCanFD_HAL::init();

    // 3) Start scheduler
    vTaskStartScheduler();

    // should never reach
    for (;;) {}
}
