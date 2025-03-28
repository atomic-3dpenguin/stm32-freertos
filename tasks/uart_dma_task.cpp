// uart_dma_task.hpp
#pragma once

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "notified_task.hpp"
#include "line_parser.hpp"
#include "tx_queue.hpp"
#include <cstring>
#include <array>

/**
 * @brief Base class for managing ISR-based UART DMA task instances.
 */
class UARTDMATaskBase {
protected:
    static constexpr std::size_t MAX_INSTANCES = 4;

    static void registerInstance(UART_HandleTypeDef* huart, void (*onTx)(), void (*onRx)());
    static void handleTxCpltCallback(UART_HandleTypeDef* huart);
    static void handleRxIdleCallback(UART_HandleTypeDef* huart);
};

/**
 * @brief CRTP-based UART DMA task using NotifiedTask, LineParser, and TxQueueManager.
 *
 * @tparam Derived User-defined class implementing onLineReceived
 */
template <typename Derived>
class UARTDMATask : public NotifiedTask<Derived>, public UARTDMATaskBase {
public:
    static constexpr std::size_t RX_BUFFER_SIZE = 256;

    UARTDMATask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
        : NotifiedTask<Derived>(name, stackSize, priority), huart(uart)
    {
        txQueue = std::make_unique<TxQueueManager>();
        registerInstance(uart, &Derived::notifyTxStatic, &Derived::notifyRxStatic);
    }

    void enqueueTxMessage(const char* msg) {
        if (txQueue->enqueue(msg) && !dmaBusy) {
            trySendNext();
        }
    }

    void notifyTx() {
        dmaBusy = false;
        this->notifyFromISR();
    }

    void notifyRx() {
        this->notifyFromISR();
    }

    static void notifyTxStatic() { instance()->notifyTx(); }
    static void notifyRxStatic() { instance()->notifyRx(); }

protected:
    /**
     * @brief Called when a line is received. Override in derived class.
     */
    virtual void onLineReceived(const char* line) {}

    /**
     * @brief Implements the NotifiedTask onNotify behavior.
     */
    void onNotify() {
        trySendNext();
        processRxData();
    }

private:
    UART_HandleTypeDef* huart;
    bool dmaBusy = false;

    std::array<char, RX_BUFFER_SIZE> rxBuffer{};
    LineParser parser;
    std::unique_ptr<TxQueueManager> txQueue;

    static auto instance() -> Derived* {
        static auto inst = Derived{};
        return &inst;
    }

    void trySendNext() {
        if (!dmaBusy && txQueue->hasMessages()) {
            std::array<char, TxQueueManager::MaxMsgSize> buffer{};
            if (txQueue->dequeue(buffer.data())) {
                dmaBusy = true;
                HAL_UART_Transmit_DMA(huart, reinterpret_cast<uint8_t*>(buffer.data()), std::strlen(buffer.data()));
            }
        }
    }

    void startRxDMA() {
        HAL_UART_Receive_DMA(huart, reinterpret_cast<uint8_t*>(rxBuffer.data()), RX_BUFFER_SIZE);
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    }

    void processRxData() {
        for (std::size_t i = 0; i < RX_BUFFER_SIZE; ++i) {
            if (parser.push(rxBuffer[i]) && parser.hasLine()) {
                static_cast<Derived*>(this)->onLineReceived(parser.getLine());
            }
        }
        startRxDMA();
    }
};
