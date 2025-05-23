// =============================
// File: can_fd_driver.hpp
// =============================
#pragma once

#include <cstdint>
#include <array>
#include "FreeRTOS.h"
#include "task.h"

namespace can {

//------------------------------------------------------------------------------
// Frame<IsFd> — fixed-size buffer, compile-time sized
//------------------------------------------------------------------------------
template<bool IsFd>
struct Frame {
    uint32_t       id;
    uint8_t        len;
    std::array<uint8_t, IsFd ? 64 : 8> data;
};

//------------------------------------------------------------------------------
// 32-bit notification flags: top-8 = DriverID, low-24 = event bits
//------------------------------------------------------------------------------
using EventFlags = uint32_t;

//------------------------------------------------------------------------------
// Driver template: glue between ControllerIF & FreeRTOS notifier
//------------------------------------------------------------------------------
template<
    typename ControllerIF,
    typename NotifierIF,
    uint8_t  DriverId,
    EventFlags EventMask
>
class Driver {
    static_assert(DriverId <= 0xFF,                   "DriverId must be 0..0xFF");
    static_assert((EventMask & 0xFF000000u) == 0,      "EventMask must fit in low-24 bits");

public:
    static constexpr EventFlags driverIdBits = (static_cast<EventFlags>(DriverId) << 24);
    static constexpr EventFlags allEvents    = driverIdBits | EventMask;

    /// Initialize controller + enable IRQs
    static void init() {
        ControllerIF::init();
        ControllerIF::enableInterrupts(EventMask);
    }

    /// Transmit a CAN or CAN-FD frame
    template<bool IsFd>
    static void send(const Frame<IsFd>& f) {
        static_assert(!IsFd || ControllerIF::supportsFd,
                      "Hardware does not support CAN-FD");
        ControllerIF::template sendFrame<IsFd>(f);
    }

    /// (Unused for HAL mode) LL-style IRQ forwarder
    static void irqHandler() {
        BaseType_t yield = pdFALSE;
        auto flags = ControllerIF::getInterruptFlags();
        ControllerIF::clearInterruptFlags(flags);

        NotifierIF::notifyFromISR(driverIdBits | (flags & EventMask), &yield);
        portYIELD_FROM_ISR(yield);
    }
};

} // namespace can
