// fdcan_tasks.cpp
#include "fdcan_tasks.hpp"
#include "logger_task.hpp"
#include <cstring>

extern LoggerTask* logger;

FDCANTask::FDCANTask(const char* name, uint16_t stackSize, UBaseType_t priority, FDCAN_HandleTypeDef* hfdcan)
    : hfdcan(hfdcan)
{
    xTaskCreate(taskEntry, name, stackSize, this, priority, &taskHandle);
    HAL_FDCAN_Start(hfdcan);
    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

void FDCANTask::notifyRx()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(taskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void FDCANTask::taskLoop()
{
    FDCAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
            if (logger) {
                logger->log(LoggerTask::LOG_INFO, "RX [CAN%lu] ID:0x%lX LEN:%lu DATA:%02X %02X %02X %02X %02X %02X %02X %02X",
                    (hfdcan->Instance == FDCAN1 ? 1UL : 2UL),
                    rxHeader.Identifier,
                    (rxHeader.DataLength >> 16) & 0xF,
                    rxData[0], rxData[1], rxData[2], rxData[3],
                    rxData[4], rxData[5], rxData[6], rxData[7]);
            }

            FDCAN_TxHeaderTypeDef txHeader;
            memcpy(&txHeader, &rxHeader, sizeof(txHeader));
            txHeader.IdType = rxHeader.IdType;
            txHeader.TxFrameType = FDCAN_DATA_FRAME;
            txHeader.DataLength = rxHeader.DataLength;
            txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
            txHeader.BitRateSwitch = FDCAN_BRS_OFF;
            txHeader.FDFormat = FDCAN_CLASSIC_CAN;
            txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
            txHeader.MessageMarker = 0;

            if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, rxData) == HAL_OK && logger) {
                logger->log(LoggerTask::LOG_INFO, "TX [CAN%lu] Echoed ID:0x%lX LEN:%lu",
                    (hfdcan->Instance == FDCAN1 ? 1UL : 2UL),
                    txHeader.Identifier,
                    (txHeader.DataLength >> 16) & 0xF);
            }
        }
    }
}

void FDCANTask::taskEntry(void* params)
{
    static_cast<FDCANTask*>(params)->taskLoop();
}

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs)
{
    extern FDCANTask* fdcan1Task;
    extern FDCANTask* fdcan2Task;

    if (hfdcan->Instance == FDCAN1) {
        if (fdcan1Task) fdcan1Task->notifyRx();
    } else if (hfdcan->Instance == FDCAN2) {
        if (fdcan2Task) fdcan2Task->notifyRx();
    }
}
