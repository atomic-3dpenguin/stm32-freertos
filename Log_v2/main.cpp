#include "LoggerMacros.hpp"

void mainTask(void*) {
    using namespace common::logger;
    Logger::instance().setOutput(uartOutput);
    Logger::instance().start();

    LOGMSG_INFO("Logger initialized");

    while (true) {
        LOGMSG_DEBUG("Loop tick %lu", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
