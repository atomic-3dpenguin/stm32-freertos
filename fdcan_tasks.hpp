// fdcan_tasks.hpp
#pragma once

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class FDCANTask {
public:
    FDCANTask(const char* name, uint16_t stackSize, UBaseType_t priority, FDCAN_HandleTypeDef* hfdcan);
    void notifyRx();

private:
    FDCAN_HandleTypeDef* hfdcan;
    TaskHandle_t taskHandle = nullptr;
    void taskLoop();
    static void taskEntry(void* params);
};
