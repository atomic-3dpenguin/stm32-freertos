#include "stm32hal.h"        // Your HAL header (defines UART_HandleTypeDef, HAL_StatusTypeDef, etc.)
#include "FreeRTOS.h"
#include "task.h"
#include "uart_parser.hpp"
#include "uart_driver.hpp"    // Your UART driver, assumed to be integrated elsewhere
#include <memory>
#include <cstring>

// For demonstration, stubs for terminal output and command handling:
#include <iostream>
void appendTerminalText(const std::string &text) {
    // In a real application, replace this with your UART transmit or logging routine.
    std::cout << text;
}

void handleCommand(const std::string &cmd) {
    // Process the complete command (e.g., parse, execute it, etc.).
    std::cout << "\nCommand received: " << cmd << std::endl;
}

// Global smart pointer for the parser.
std::unique_ptr<UartParser> g_uartParser;

//---------------------------------------------------------------------
// DMA Callback: Called when the UART DMA event completes.
// This pushes new data from the DMA buffer into the circular buffer.
//---------------------------------------------------------------------
extern "C" void UART_DMA_EventCompleteCallback() {
    // In your system these variables should point to the DMA buffer and its length.
    extern const uint8_t* dmaBuffer;
    extern std::size_t dmaBufferLength;

    // Push the newly received data into our parser’s circular buffer.
    g_uartParser->pushData(dmaBuffer, dmaBufferLength);
}

//---------------------------------------------------------------------
// FreeRTOS Task: UartParserTask waits for new data and processes it.
//---------------------------------------------------------------------
extern "C" void UartParserTask(void* pvParameters) {
    for (;;) {
        // In this example, we simply delay and then process any buffered data.
        // You may also combine this with a notification mechanism from the DMA callback.
        vTaskDelay(pdMS_TO_TICKS(50));
        g_uartParser->process();
    }
}

//---------------------------------------------------------------------
// Main Function: Initializes HAL, creates the parser, and starts tasks.
//---------------------------------------------------------------------
int main(void) {
    // Initialize HAL.
    HAL_Init();

    // Perform system clock and peripheral initialization as needed.
    // For instance, SystemClock_Config(); MX_USART1_UART_Init(); etc.

    // Create our UartParser instance managed by a smart pointer.
    g_uartParser = std::make_unique<UartParser>();

    // (Optionally) Create and initialize your UART driver and register callbacks.
    // For example, assume you create a UartDriver instance via a smart pointer.
    // UartDriverPtr uart = std::make_unique<UartDriver>(&huart1);
    // uart->registerCallback(HAL_UART_RX_COMPLETE_CB_ID, UartRxCallback);
    // uart->receive(...);

    // Create the FreeRTOS task that processes incoming UART data.
    TaskHandle_t xParserTaskHandle = nullptr;
    if (xTaskCreate(UartParserTask, "UART_Parser", 256, nullptr, tskIDLE_PRIORITY + 1, &xParserTaskHandle) != pdPASS) {
        // Handle task creation failure (e.g., loop indefinitely).
        while (1);
    }

    // Start the FreeRTOS scheduler.
    vTaskStartScheduler();

    // Should never get here.
    while(1);
    return 0;
}
