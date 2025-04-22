#pragma once

#include <cstdint>

namespace lp5861_defs {

/**
 * @brief PWM output frequency selections for LP5861
 * 
 * The LP5861 supports two audible‑noise‑free PWM frequencies:
 *  - 125 kHz
 *  -  62.5 kHz
 * 
 * fPWM is selected by writing one of these values into REG_PWM_FREQ (0x1E). :contentReference[oaicite:0]{index=0}
 */
enum class PWMFrequency : uint8_t {
    FREQ_125KHZ = 0x00,   ///< 125 kHz PWM output
    FREQ_62_5KHZ = 0x01   ///<  62.5 kHz PWM output
};

/**
 * @brief Offset of each color channel register from the LED‑n base
 * 
 * In the LP5861, each RGB LED has three consecutive registers:
 *   Base + 0 → Blue  
 *   Base + 1 → Green  
 *   Base + 2 → Red  
 * 
 * Using these enums makes the write offsets self‑documenting.
 */
enum class ColorChannel : uint8_t {
    Blue  = 0,  ///< Blue channel register offset
    Green = 1,  ///< Green channel register offset
    Red   = 2   ///< Red channel register offset
};

/**
 * @brief Simple LED index enumeration (0…5 for 6 RGB LEDs)
 */
enum class LEDIndex : uint8_t {
    LED0 = 0,
    LED1 = 1,
    LED2 = 2,
    LED3 = 3,
    LED4 = 4,
    LED5 = 5
};

} // namespace lp5861_defs

// #include "LP5861Interface.hpp"
// #include "LP5861Defs.hpp"

// // ...

// // Set 62.5 kHz PWM globally:
// ledDriver.setFrequency(static_cast<uint8_t>(lp5861_defs::PWMFrequency::FREQ_62_5KHZ));

// // Set LED2 to full green:
// uint8_t base = REG_LED0_B + 
//                static_cast<uint8_t>(lp5861_defs::LEDIndex::LED2) * 3;
// ledDriver.writeReg(base + static_cast<uint8_t>(lp5861_defs::ColorChannel::Green), 0xFF);
