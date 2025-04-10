#include "uart_driver.hpp"

// The constructor simply passes the handle to the base class.
UartDriver::UartDriver(UART_HandleTypeDef* huart)
    : CallbackDriver(huart)
{
}

// Wrap the HAL transmit function in interrupt mode.
HAL_StatusTypeDef UartDriver::transmit(uint8_t* data, uint16_t length) {
    return HAL_UART_Transmit_IT(handle_, data, length);
}

// Wrap the HAL receive function in interrupt mode.
HAL_StatusTypeDef UartDriver::receive(uint8_t* buffer, uint16_t length) {
    return HAL_UART_Receive_IT(handle_, buffer, length);
}
