// fdcan_task.hpp
#pragma once

#include "notified_task.hpp"
#include "stm32f4xx_hal.h"
#include <array>
#include <cstring>

/**
 * @brief FDCAN Task using CRTP-based NotifiedTask
 */
class FDCANTask : public NotifiedTask<FDCANTask> {
public:
    FDCANTask(const char* name, uint16_t stackSize, UBaseType_t priority, FDCAN_HandleTypeDef* handle)
        : NotifiedTask<FDCANTask>(name, stackSize, priority), hfdcan(handle)
    {
        HAL_FDCAN_Start(hfdcan);
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    }

    void notifyRxFromISR() {
        this->notifyFromISR();
    }

    void onNotify() {
        FDCAN_RxHeaderTypeDef rxHeader;
        std::array<uint8_t, 8> rxData{};

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData.data()) == HAL_OK) {
            FDCAN_TxHeaderTypeDef txHeader = rxHeader;
            txHeader.TxFrameType = FDCAN_DATA_FRAME;
            txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
            txHeader.MessageMarker = 0;

            HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, rxData.data());
        }
    }

private:
    FDCAN_HandleTypeDef* hfdcan;
};

extern FDCANTask* fdcan1Task;
extern FDCANTask* fdcan2Task;

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
    if (hfdcan->Instance == FDCAN1 && fdcan1Task) {
        fdcan1Task->notifyRxFromISR();
    } else if (hfdcan->Instance == FDCAN2 && fdcan2Task) {
        fdcan2Task->notifyRxFromISR();
    }
}
