#include "stm32hal.h"        // HAL header for initialization and UART definitions.
#include "FreeRTOS.h"
#include "task.h"
#include "uart_driver.hpp"    // Include our UART driver.
#include <cstring>
#include <memory>

// Global smart pointer to our UartDriver instance.
static UartDriverPtr g_uartDriver = nullptr;

// FreeRTOS task handle for the UART task.
TaskHandle_t xUartTaskHandle = nullptr;

// Global volatile variable to hold the received byte. This is updated by the HAL in interrupt context.
volatile uint8_t rxByte = 0;

//---------------------------------------------------------------------
// UART RX Callback - Called from HAL interrupt context.
// This function sends a task notification with the received byte.
//---------------------------------------------------------------------
extern "C" void UartRxCallback(UART_HandleTypeDef* huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Pass the received byte via task notification (as a 32-bit value).
    xTaskNotifyFromISR(xUartTaskHandle, static_cast<uint32_t>(rxByte), eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//---------------------------------------------------------------------
// UART Task - Processes received data using FreeRTOS task notifications.
//---------------------------------------------------------------------
extern "C" void UartTask(void* pvParameters) {
    uint32_t notificationValue = 0;
    for (;;) {
        // Wait indefinitely for a notification from the ISR.
        if (xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY) == pdTRUE) {
            // Retrieve the received byte from the notification.
            uint8_t receivedByte = static_cast<uint8_t>(notificationValue);

            // Echo the received byte back.
            g_uartDriver->transmit(&receivedByte, 1);

            // Re-arm the reception for the next byte.
            g_uartDriver->receive(const_cast<uint8_t*>(&rxByte), 1);
        }
    }
}

//---------------------------------------------------------------------
// Main function: Initializes HAL, creates the UART driver, registers the callback,
// creates the FreeRTOS task, and starts the scheduler.
//---------------------------------------------------------------------
int main(void) {
    // Initialize the HAL.
    HAL_Init();

    // --- System Clock & Peripheral Configuration ---
    // Ensure that the system clock and UART peripherals are configured.
    // For CubeMX projects, you might call SystemClock_Config() and MX_USART1_UART_Init(), etc.

    // Initialize the UART handle (example for UART1).
    UART_HandleTypeDef huart1 = {};  // Zero-initialize the structure.
    // TODO: Populate huart1 with proper parameters or call CubeMX generated init function.

    // Create the UartDriver instance using a smart pointer.
    g_uartDriver = std::make_unique<UartDriver>(&huart1);

    // Register the UART receive complete callback.
    if (g_uartDriver->registerCallback(HAL_UART_RX_COMPLETE_CB_ID, UartRxCallback) != HAL_OK) {
        // Handle error (for example, loop indefinitely).
        while (1);
    }

    // Start the initial reception in interrupt mode.
    if (g_uartDriver->receive(const_cast<uint8_t*>(&rxByte), 1) != HAL_OK) {
        while (1);
    }

    // Optionally, send a welcome message.
    const char* welcomeMsg = "UART Ready\r\n";
    if (g_uartDriver->transmit(reinterpret_cast<uint8_t*>(const_cast<char*>(welcomeMsg)), std::strlen(welcomeMsg)) != HAL_OK) {
        while (1);
    }

    // Create the UART FreeRTOS task.
    if (xTaskCreate(UartTask, "UART_Task", 256, nullptr, tskIDLE_PRIORITY + 1, &xUartTaskHandle) != pdPASS) {
        // Error creating the task.
        while (1);
    }

    // Start the FreeRTOS scheduler.
    vTaskStartScheduler();

    // The scheduler should never return here.
    while (1);
    return 0;
}
