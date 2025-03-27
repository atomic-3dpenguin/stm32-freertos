// logger_task.cpp
#include "logger_task.hpp"
#include <cstdio>
#include <cstring>

LoggerTask::LoggerTask(const char* name, uint16_t stackSize, UBaseType_t priority, UART_HandleTypeDef* uart)
    : UARTDMATask(name, stackSize, priority, uart)
{}

void LoggerTask::log(LogLevel level, const char* format, ...)
{
    char buffer[128];
    int offset = snprintf(buffer, sizeof(buffer), "%s", levelPrefixes[level]);

    va_list args;
    va_start(args, format);
    vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);

    strncat(buffer, "\r\n", sizeof(buffer) - strlen(buffer) - 1);
    enqueueTxMessage(buffer);
}
