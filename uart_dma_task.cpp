// uart_dma_task.cpp
#include "uart_dma_task.hpp"
#include <cstring>

UARTDMATask* UARTDMATask::instances[MAX_INSTANCES] = {nullptr};

UARTDMATask::UARTDMATask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
    : huart(uart)
{
    txQueue = xQueueCreate(QUEUE_LENGTH, MAX_MSG_SIZE);
    registerInstance(this);
    xTaskCreate(taskEntry, name, stackSize, this, priority, &taskHandle);
}

void UARTDMATask::registerInstance(UARTDMATask* instance)
{
    for (int i = 0; i < MAX_INSTANCES; ++i) {
        if (!instances[i]) {
            instances[i] = instance;
            return;
        }
    }
}

void UARTDMATask::enqueueTxMessage(const char* msg)
{
    if (msg && strlen(msg) < MAX_MSG_SIZE) {
        xQueueSend(txQueue, msg, 0);
        if (!dmaBusy) {
            trySendNext();
        }
    }
}

void UARTDMATask::trySendNext()
{
    if (!dmaBusy && uxQueueMessagesWaiting(txQueue) > 0) {
        if (xQueueReceive(txQueue, dmaTxBuffer, 0) == pdTRUE) {
            dmaBusy = true;
            HAL_UART_Transmit_DMA(huart, (uint8_t*)dmaTxBuffer, strlen(dmaTxBuffer));
        }
    }
}

void UARTDMATask::notifyTxCompleteFromISR()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    dmaBusy = false;
    vTaskNotifyGiveFromISR(taskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void UARTDMATask::notifyRxIdleFromISR()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(taskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void UARTDMATask::startRxDMA()
{
    HAL_UART_Receive_DMA(huart, (uint8_t*)rxBuffer, RX_BUFFER_SIZE);
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
}

void UARTDMATask::processRxData()
{
    // Find newline
    for (size_t i = 0; i < RX_BUFFER_SIZE; ++i) {
        if (rxBuffer[i] == '\n' || rxBuffer[i] == '\r') {
            rxBuffer[i] = '\0';
            onLineReceived(rxBuffer);
            break;
        }
    }
    startRxDMA();
}

void UARTDMATask::taskLoop()
{
    startRxDMA();
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        trySendNext();
        processRxData();
    }
}

void UARTDMATask::taskEntry(void* params)
{
    static_cast<UARTDMATask*>(params)->taskLoop();
}

void UARTDMATask::handleTxCpltCallback(UART_HandleTypeDef* huart)
{
    for (int i = 0; i < MAX_INSTANCES; ++i) {
        if (instances[i] && instances[i]->huart == huart) {
            instances[i]->notifyTxCompleteFromISR();
            break;
        }
    }
}

void UARTDMATask::handleRxIdleCallback(UART_HandleTypeDef* huart)
{
    __HAL_UART_CLEAR_IDLEFLAG(huart);
    for (int i = 0; i < MAX_INSTANCES; ++i) {
        if (instances[i] && instances[i]->huart == huart) {
            instances[i]->notifyRxIdleFromISR();
            break;
        }
    }
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    UARTDMATask::handleTxCpltCallback(huart);
}

extern "C" void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart2);
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)) {
        UARTDMATask::handleRxIdleCallback(&huart2);
    }
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    // Not used with idle line DMA; kept for compatibility if needed
}
