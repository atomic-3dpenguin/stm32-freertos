// system_manager.cpp
#include "system_manager.hpp"
#include "cli_commands.hpp"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

std::unique_ptr<FDCANTask> fdcan1Task;
std::unique_ptr<FDCANTask> fdcan2Task;

extern "C" void SystemClock_Config(void);
extern "C" void MX_GPIO_Init(void);
extern "C" void MX_USART2_UART_Init(void);
extern "C" void MX_FDCAN1_Init(void);
extern "C" void MX_FDCAN2_Init(void);

void SystemManager::init() {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_FDCAN1_Init();
    MX_FDCAN2_Init();

    logger = std::make_unique<LoggerTask>("Logger", 512, osPriorityNormal, &huart2);
    cli = std::make_unique<CLITask>("CLI", 512, osPriorityNormal, &huart2);
    fdcan1Task = std::make_unique<FDCANTask>("FDCAN1", 512, osPriorityNormal, &hfdcan1);
    fdcan2Task = std::make_unique<FDCANTask>("FDCAN2", 512, osPriorityNormal, &hfdcan2);

    cli->setLogger(logger.get());
    registerDefaultCLICommands(cli->getRegistry());
}

void SystemManager::run() {
    vTaskStartScheduler();
    while (1) {}
}
