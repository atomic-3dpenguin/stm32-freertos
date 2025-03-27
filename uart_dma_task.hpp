// uart_dma_task.hpp
#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

class UARTDMATask {
public:
    UARTDMATask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart);
    virtual ~UARTDMATask() = default;

    void start();
    void notifyTxCompleteFromISR();
    void notifyRxIdleFromISR();

protected:
    virtual void onLineReceived(const char* line) {} // For CLI overrides
    virtual void onTxQueueEmpty() {}                // For Logger overrides

    void enqueueTxMessage(const char* msg);
    void processRxData();

    static void handleTxCpltCallback(UART_HandleTypeDef* huart);
    static void handleRxIdleCallback(UART_HandleTypeDef* huart);

private:
    static constexpr size_t QUEUE_LENGTH = 16;
    static constexpr size_t MAX_MSG_SIZE = 128;
    static constexpr size_t RX_BUFFER_SIZE = 256;
    static constexpr int MAX_INSTANCES = 4;

    static UARTDMATask* instances[MAX_INSTANCES];
    static void registerInstance(UARTDMATask* instance);

    UART_HandleTypeDef* huart;
    TaskHandle_t taskHandle = nullptr;
    QueueHandle_t txQueue;

    char rxBuffer[RX_BUFFER_SIZE];
    size_t rxWritePos = 0;

    char dmaTxBuffer[MAX_MSG_SIZE];
    bool dmaBusy = false;

    void taskLoop();
    void startRxDMA();
    void trySendNext();

    static void taskEntry(void* params);
};
