/*
 * cli_common.hpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

#ifndef CLI_COMMON_HPP_
#define CLI_COMMON_HPP_

#include <cstdint>

namespace cli {

using CommandId = uint32_t;

// Bit layout: bits 31-24: subsystem (8 bits), bits 23-0: command id (24 bits)
constexpr uint8_t SUBSYS_SHIFT = 24;
constexpr CommandId SUBSYS_MASK = static_cast<CommandId>(0xFF) << SUBSYS_SHIFT;
constexpr CommandId CMD_MASK   = ~SUBSYS_MASK;

// Create CommandId from raw values
constexpr CommandId makeCmd(uint8_t subsys, uint32_t cmd) {
    return (static_cast<CommandId>(subsys) << SUBSYS_SHIFT) | (cmd & CMD_MASK);
}

// Strongly-typed enums
enum class Subsystem : uint8_t {
    CORE = 0,
    LED  = 1,
    // ...
};

enum class CoreCmd : uint32_t {
    Help = 0,
};

enum class LedCmd : uint32_t {
    On  = 1,
    Off = 2,
    RGB = 3,
};

// Overload for enum classes
template<typename CmdEnum>
constexpr CommandId makeCmd(Subsystem s, CmdEnum c) {
    return makeCmd(static_cast<uint8_t>(s), static_cast<uint32_t>(c));
}

} // namespace cli



#endif /* CLI_COMMON_HPP_ */
