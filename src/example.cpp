#include "stm32hal.h"      // Replace with your actual HAL header
#include "FreeRTOS.h"
#include "task.h"
#include <cstring>

//---------------------------------------------------------------------
// Generic CRTP Callback Driver Template (from previous example)
//---------------------------------------------------------------------

template<
    typename Derived,
    typename HandleType,
    typename CBIDType,
    typename CallbackType,
    HAL_StatusTypeDef (*RegFunc)(HandleType*, CBIDType, CallbackType),
    HAL_StatusTypeDef (*UnregFunc)(HandleType*, CBIDType)
>
class CallbackDriver {
public:
    // Accept the driver handle.
    explicit CallbackDriver(HandleType* handle)
        : handle_(handle) {}

    // Register a callback with the underlying C HAL.
    HAL_StatusTypeDef registerCallback(CBIDType cbId, CallbackType callback) {
        return RegFunc(handle_, cbId, callback);
    }

    // Unregister a callback.
    HAL_StatusTypeDef unregisterCallback(CBIDType cbId) {
        return UnregFunc(handle_, cbId);
    }

protected:
    HandleType* handle_;
};

//---------------------------------------------------------------------
// UART Driver specialization using our generic template.
//---------------------------------------------------------------------

class UartDriver : public CallbackDriver<
    UartDriver,                // CRTP Derived class
    UART_HandleTypeDef,        // Handle type
    HAL_UART_CallbackIDTypeDef,// Callback ID type
    pUART_CallbackTypeDef,     // Callback function pointer type
    HAL_UART_RegisterCallback, // Registration function from HAL
    HAL_UART_UnRegisterCallback// Unregistration function from HAL
>
{
public:
    // Pass the UART handle to the base.
    explicit UartDriver(UART_HandleTypeDef* huart)
        : CallbackDriver(huart) {}

    // Non-blocking transmit (using interrupt-based transmission).
    HAL_StatusTypeDef transmit(uint8_t* data, uint16_t length) {
        return HAL_UART_Transmit_IT(handle_, data, length);
    }

    // Non-blocking receive.
    HAL_StatusTypeDef receive(uint8_t* buffer, uint16_t length) {
        return HAL_UART_Receive_IT(handle_, buffer, length);
    }
};

//---------------------------------------------------------------------
// Global Declarations for FreeRTOS integration.
//---------------------------------------------------------------------

// Global pointer to our UART driver, accessible by both the task and callback.
UartDriver* g_uartDriver = nullptr;

// Global task handle for the UART task.
TaskHandle_t xUartTaskHandle = nullptr;

// Global variable to hold the received byte (used by HAL to write the incoming data).
// Declared as volatile since it is modified in interrupt context.
volatile uint8_t rxByte = 0;

//---------------------------------------------------------------------
// UART RX Callback - Called from HAL interrupt context.
//---------------------------------------------------------------------

void UartRxCallback(UART_HandleTypeDef* huart) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // Pass the received byte via task notification using eSetValueWithOverwrite.
    // This writes the rxByte value (converted to uint32_t) directly as notification value.
    xTaskNotifyFromISR(xUartTaskHandle, static_cast<uint32_t>(rxByte), eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    // Request a context switch if needed.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

//---------------------------------------------------------------------
// UART Task - Processes received data using task notifications.
//---------------------------------------------------------------------

void UartTask(void* pvParameters) {
    uint32_t notificationValue = 0;
    for (;;) {
        // Wait indefinitely for a notification from the ISR callback.
        if (xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY) == pdTRUE) {
            // Retrieve the received byte from the notification value.
            uint8_t receivedByte = static_cast<uint8_t>(notificationValue);

            // Echo the received byte back.
            g_uartDriver->transmit(&receivedByte, 1);

            // Re-arm reception for the next byte.
            // It is important to call this once the previous reception is processed.
            g_uartDriver->receive(const_cast<uint8_t*>(&rxByte), 1);
        }
    }
}

//---------------------------------------------------------------------
// Main Function - Initializes hardware, configures the UART driver,
// registers the callback, creates the UART FreeRTOS task, and starts
// the scheduler.
//---------------------------------------------------------------------

int main(void)
{
    // Initialize HAL.
    HAL_Init();

    // --- System Clock and Peripheral Configuration ---
    // (Assume proper configuration functions are called here)
    
    // Initialize your UART handle.
    // For example, assuming using UART1:
    UART_HandleTypeDef huart1 = {};   // Zero-initialize the handle structure.
    // TODO: Populate huart1 with required settings and call initialization routines,
    // e.g., MX_USART1_UART_Init() if using CubeMX generated code.
    
    // Create an instance of the UART driver and assign it to the global pointer.
    UartDriver uart(&huart1);
    g_uartDriver = &uart;

    // Register the UART receive complete callback.
    if (uart.registerCallback(HAL_UART_RX_COMPLETE_CB_ID, UartRxCallback) != HAL_OK) {
         // Handle error (for demonstration, loop indefinitely).
         while(1);
    }

    // Start initial reception in interrupt mode.
    if (uart.receive(const_cast<uint8_t*>(&rxByte), 1) != HAL_OK) {
         // Handle error (for demonstration, loop indefinitely).
         while(1);
    }

    // Optionally, send a welcome message.
    const char* welcomeMsg = "UART Ready\r\n";
    if (uart.transmit((uint8_t*)welcomeMsg, std::strlen(welcomeMsg)) != HAL_OK) {
         while(1);
    }

    // Create the UART FreeRTOS task.
    // Stack size and priority can be adjusted as needed.
    BaseType_t result = xTaskCreate(UartTask, "UART_Task", 256, nullptr, tskIDLE_PRIORITY + 1, &xUartTaskHandle);
    if (result != pdPASS) {
         // Error creating the task.
         while(1);
    }

    // Start the FreeRTOS scheduler.
    vTaskStartScheduler();

    // The program should never reach here.
    for (;;);
    return 0;
}
