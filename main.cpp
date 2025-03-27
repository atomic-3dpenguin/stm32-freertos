// main.cpp
#include "logger_task.hpp"
#include "cli_task.hpp"
#include "cli_commands.hpp"

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// UART handles (externally defined or initialized elsewhere)
extern UART_HandleTypeDef huart2;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

FDCANTask* fdcan1Task = nullptr;
FDCANTask* fdcan2Task = nullptr;
LoggerTask* logger = nullptr;
CLITask* cli = nullptr;

extern "C" void SystemClock_Config(void);
extern "C" void MX_GPIO_Init(void);
extern "C" void MX_USART2_UART_Init(void);

int main()
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    static LoggerTask loggerTask("Logger", 512, osPriorityNormal, &huart2);
    static CLITask cliTask("CLI", 512, osPriorityNormal, &huart2);
    fdcan1Task = new FDCANTask("FDCAN1", 256, osPriorityNormal, &hfdcan1);
    fdcan2Task = new FDCANTask("FDCAN2", 256, osPriorityNormal, &hfdcan2);

    logger = &loggerTask;
    cli = &cliTask;

    cli->setLogger(logger);

    // Register commands
    cli->registerCommand("help", HelpCommand::executor);
    cli->registerCommand("echo", EchoCommand::executor);
    cli->registerCommand("add", AddCommand::executor);
    cli->registerCommand("setled", SetLedCommand::executor);
    cli->registerCommand("tab", TabCommand::executor);
    cli->registerCommand("cansend", CanSendCommand::executor);
    cli->registerCommand("canstat", CanStatCommand::executor);


    vTaskStartScheduler();

    while (1) {} // Should never reach here
}
