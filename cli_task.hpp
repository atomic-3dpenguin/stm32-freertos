// cli_task.hpp
#pragma once

#include "uart_dma_task.hpp"
#include <cstring>

class CLITask : public UARTDMATask {
public:
    CLITask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart);

protected:
    void onLineReceived(const char* line) override;

private:
    void handleCommand(const char* cmd);
};
