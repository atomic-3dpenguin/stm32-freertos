// ===========================================
// File: can_fd_hal_callbacks.cpp
// ===========================================
#include "can_fd_driver.hpp"
#include "can_fd_hal_if.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"            // if you use CMSIS-RTOS wrappers

//------------------------------------------------------------------------------
// User-provided FreeRTOS task handle
TaskHandle_t CanTaskHandle = nullptr;

//------------------------------------------------------------------------------
// NotifierIF — wraps xTaskNotifyFromISR
struct CanTaskNotifier {
    static void notifyFromISR(can::EventFlags flags, BaseType_t* pYield) {
        xTaskNotifyFromISR(CanTaskHandle, flags, eSetBits, pYield);
    }
};

//------------------------------------------------------------------------------
// Event bits (low-24) — pick your own
enum : can::EventFlags {
    Event_RxPending  = (1u << 0),
    Event_TxComplete = (1u << 1),
    Event_Error      = (1u << 2),
};

//------------------------------------------------------------------------------
// Instantiate the driver: ID=1, want Rx,Tx,Error
using MyCanFD_HAL = can::Driver<
    FDCAN1_HAL_IF,
    CanTaskNotifier,
    /*DriverId=*/1,
    Event_RxPending | Event_TxComplete | Event_Error
>;

//------------------------------------------------------------------------------
// Override HAL weak callbacks:
extern "C" void HAL_FDCAN_RxFifo0MsgPendingCallback(FDCAN_HandleTypeDef* hfdcan) {
    if (hfdcan->Instance == FDCAN1) {
        __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_RXF0N);
        BaseType_t y = pdFALSE;
        CanTaskNotifier::notifyFromISR(
            MyCanFD_HAL::driverIdBits | Event_RxPending, &y
        );
        portYIELD_FROM_ISR(y);
    }
}

extern "C" void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t BufferIdx) {
    if (hfdcan->Instance == FDCAN1) {
        __HAL_FDCAN_CLEAR_FLAG(hfdcan, FDCAN_FLAG_TE);
        BaseType_t y = pdFALSE;
        CanTaskNotifier::notifyFromISR(
            MyCanFD_HAL::driverIdBits | Event_TxComplete, &y
        );
        portYIELD_FROM_ISR(y);
    }
}

extern "C" void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef* hfdcan) {
    if (hfdcan->Instance == FDCAN1) {
        uint32_t err = HAL_FDCAN_GetError(&FDCAN1_HAL_IF::hfdcan1);
        BaseType_t y = pdFALSE;
        CanTaskNotifier::notifyFromISR(
            MyCanFD_HAL::driverIdBits | (err & Event_Error), &y
        );
        portYIELD_FROM_ISR(y);
    }
}
