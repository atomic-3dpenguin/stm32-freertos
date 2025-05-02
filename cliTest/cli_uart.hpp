/*
 * cli_uart.hpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

#ifndef CLI_UART_HPP_
#define CLI_UART_HPP_

#include  "cli_base.hpp"
#include "stm32f7xx_hal.h"

namespace cli::transport {

class CLIUART : public cli::CLIBase<CLIUART> {
public:
    CLIUART(UART_HandleTypeDef* hu, const std::array<osEventFlagsId_t,8>& evts)
      : CLIBase(evts), huart_(hu) {}

    char getCharImpl() {
        uint8_t c;
        HAL_UART_Receive(huart_, &c, 1, HAL_MAX_DELAY);
        return c;
    }
    void putCharImpl(char c) {
        uint8_t d = static_cast<uint8_t>(c);
        HAL_UART_Transmit(huart_, &d, 1, HAL_MAX_DELAY);
    }

private:
    UART_HandleTypeDef* huart_;
};

} // namespace cli::transport

#endif /* CLI_UART_HPP_ */
