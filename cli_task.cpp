// cli_task.cpp
#include "cli_task.hpp"
#include "logger_task.hpp"
#include <cstdio>
#include <cstring>

LoggerTask* globalLogger = nullptr;

CLITask::CLITask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
    : UARTDMATask(name, stackSize, priority, uart) {}

void CLITask::setLogger(LoggerTask* logger) {
    globalLogger = logger;
}

void CLITask::onLineReceived(const char* line) {
    handleCommand(line);
}

void CLITask::handleCommand(const char* input) {
    char response[128];
    char commandName[32];
    const char* args = strchr(input, ' ');

    if (args) {
        size_t len = args - input;
        strncpy(commandName, input, len);
        commandName[len] = '\0';
        ++args; // move past space
    } else {
        strncpy(commandName, input, sizeof(commandName) - 1);
        commandName[sizeof(commandName) - 1] = '\0';
        args = "";
    }

    for (int i = 0; i < MAX_COMMANDS; ++i) {
        if (strcmp(commandTable[i].name, commandName) == 0) {
            commandTable[i].execute(args, response, sizeof(response));
            enqueueTxMessage(response);
            if (globalLogger) globalLogger->log(LoggerTask::LOG_INFO, "Executed command: %s", commandName);
            return;
        }
    }

    snprintf(response, sizeof(response), "Unknown command: %s\r\n", commandName);
    enqueueTxMessage(response);
    if (globalLogger) globalLogger->log(LoggerTask::LOG_WARN, "Unknown command received: %s", commandName);
}

// CRTP command wrapper

#include "cli_command_base.hpp"

void CLITask::registerCommand(const char* name, void (*executor)(const char*, char*, size_t)) {
    for (int i = 0; i < MAX_COMMANDS; ++i) {
        if (commandTable[i].name[0] == '\0') {
            strncpy(commandTable[i].name, name, sizeof(commandTable[i].name) - 1);
            commandTable[i].execute = executor;
            return;
        }
    }
    if (globalLogger) globalLogger->log(LoggerTask::LOG_WARN, "Command table full, cannot register: %s", name);
}
