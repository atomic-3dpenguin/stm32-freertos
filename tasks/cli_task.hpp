// cli_task.hpp
#pragma once

#include "uart_dma_task.hpp"
#include "logger_task.hpp"
#include "cli_registry.hpp"
#include <cstring>

/**
 * @brief CLI task handling UART input using CRTP UARTDMATask.
 */
class CLITask : public UARTDMATask<CLITask> {
public:
    CLITask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
        : UARTDMATask(name, stackSize, priority, uart) {}

    void setLogger(LoggerTask* loggerPtr) {
        logger = loggerPtr;
    }

    void registerCommand(std::string_view name, CLIRegistry::CommandHandler handler) {
        registry.registerCommand(name, std::move(handler));
    }

    CLIRegistry& getRegistry() {
        return registry;
    }

protected:
    void onLineReceived(const char* line) override {
        handleCommand(line);
    }

private:
    LoggerTask* logger = nullptr;
    CLIRegistry registry;

    void handleCommand(const char* cmd) {
        auto response = registry.execute(cmd);
        enqueueTxMessage(response.c_str());
        if (logger) logger->log(LoggerTask::LOG_INFO, "Executed: %s", cmd);
    }
};
