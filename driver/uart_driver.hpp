#ifndef UART_DRIVER_HPP
#define UART_DRIVER_HPP

#include "callback_driver.hpp"
#include "stm32hal.h"
#include <memory>

// UartDriver class that derives from our generic CallbackDriver.
// Note that the template parameters are filled with UART-specific types and HAL functions.
class UartDriver : public CallbackDriver<
    UartDriver,                  // CRTP-derived class
    UART_HandleTypeDef,          // Handle type for UART
    HAL_UART_CallbackIDTypeDef,  // Callback ID type
    pUART_CallbackTypeDef,       // Callback function type for UART
    HAL_UART_RegisterCallback,   // HAL function for callback registration
    HAL_UART_UnRegisterCallback  // HAL function for callback unregistration
>
{
public:
    // Constructor: pass the UART handle to the base class.
    explicit UartDriver(UART_HandleTypeDef* huart);

    // Non-blocking transmit wrapper.
    HAL_StatusTypeDef transmit(uint8_t* data, uint16_t length);

    // Non-blocking receive wrapper.
    HAL_StatusTypeDef receive(uint8_t* buffer, uint16_t length);
};

// Define an alias for a unique pointer to UartDriver.
using UartDriverPtr = std::unique_ptr<UartDriver>;

#endif // UART_DRIVER_HPP
