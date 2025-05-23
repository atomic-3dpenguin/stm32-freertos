// =============================
// File: can_fd_hal_if.hpp
// =============================
#pragma once

#include "stm32g4xx_hal.h"
#include "can_fd_driver.hpp"

/// HAL-backed ControllerIF for FDCAN1
struct FDCAN1_HAL_IF {
    static constexpr bool supportsFd = true;
    static inline FDCAN_HandleTypeDef hfdcan1;

    static void init() {
        // Either call the MX init or manually:
        extern void MX_FDCAN1_Init();  
        MX_FDCAN1_Init();
    }

    template<bool IsFd>
    static void sendFrame(const can::Frame<IsFd>& f) {
        FDCAN_TxHeaderTypeDef hdr = {};
        hdr.Identifier        = f.id;
        hdr.IdType            = FDCAN_STANDARD_ID;
        hdr.DataLength        = f.len;
        hdr.ErrorStateMask    = FDCAN_ERROR_STATE_PASSED;
        hdr.TxEventFifoControl= FDCAN_NO_TX_EVENTS;

        if constexpr (IsFd) {
            hdr.FDFormat      = FDCAN_FD_CAN;
            hdr.BitRateSwitch = FDCAN_BRS_ON;
        } else {
            hdr.FDFormat      = FDCAN_CLASSIC_CAN;
            hdr.BitRateSwitch = FDCAN_BRS_OFF;
        }

        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &hdr, f.data.data());
    }
};
