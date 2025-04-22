#pragma once

#include "stm32g4xx_hal.h"
#include <cstdint>

namespace drivers {

/**
 * @brief CRTP-based interface for the TI LP5861 18-Channel LED Driver
 * @tparam Derived A class providing getI2cHandle() and getDeviceAddress()
 */
template <typename Derived>
class LP5861Interface {
public:
    using Status = HAL_StatusTypeDef;

    /**
     * @brief Set the global PWM frequency
     * @param freqReg 8-bit register value controlling frequency
     * @return HAL status
     */
    Status setFrequency(uint8_t freqReg) {
        return writeReg(REG_PWM_FREQ, freqReg);
    }

    /**
     * @brief Set the color of one RGB LED (0..5)
     * @param ledIndex LED index [0..5]
     * @param red   Red brightness (0..255)
     * @param green Green brightness (0..255)
     * @param blue  Blue brightness (0..255)
     * @return HAL status
     */
    Status setColor(uint8_t ledIndex, uint8_t red, uint8_t green, uint8_t blue) {
        if (ledIndex >= LED_COUNT) {
            return HAL_ERROR;
        }
        // LP5861 LEDn registers are in order: B, G, R
        uint8_t base = REG_LED0_B + ledIndex * 3;
        Status s = writeReg(base + 2, red);
        if (s != HAL_OK) return s;
        s = writeReg(base + 1, green);
        if (s != HAL_OK) return s;
        return writeReg(base + 0, blue);
    }

protected:
    static constexpr uint8_t REG_PWM_FREQ = 0x1E;  ///< PWM frequency control register
    static constexpr uint8_t REG_LED0_B   = 0x24;  ///< LED0 Blue register
    static constexpr uint8_t LED_COUNT    = 6;     ///< Number of RGB LEDs

    /**
     * @brief Low-level register write over I2C
     * @param reg   Register address
     * @param value Byte to write
     * @return HAL status
     */
    Status writeReg(uint8_t reg, uint8_t value) {
        return HAL_I2C_Mem_Write(
            derived().getI2cHandle(),
            derived().getDeviceAddress(),
            reg,
            I2C_MEMADD_SIZE_8BIT,
            &value,
            1,
            HAL_MAX_DELAY
        );
    }

    /**
     * @brief Access the derived class
     */
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }
};

} // namespace drivers


/**
 * @brief Example concrete driver class
 */
class LP5861 : public drivers::LP5861Interface<LP5861> {
public:
    /**
     * @param hi2c         Pointer to HAL I2C handle
     * @param devAddr7bit  7-bit I2C address of LP5861
     */
    LP5861(I2C_HandleTypeDef* hi2c, uint16_t devAddr7bit)
        : hi2c_(hi2c)
        , addr7bit_(devAddr7bit)
    {}

    I2C_HandleTypeDef* getI2cHandle() const { return hi2c_; }
    uint16_t getDeviceAddress()  const { return addr7bit_ << 1; }

private:
    I2C_HandleTypeDef* hi2c_;
    uint16_t           addr7bit_;
};

// Usage example:
// extern I2C_HandleTypeDef hi2c1;
// LP5861 ledDriver(&hi2c1, 0x30);
// ledDriver.setFrequency(0x0F);
// ledDriver.setColor(0, 255, 0, 0);  // LED0 -> red
