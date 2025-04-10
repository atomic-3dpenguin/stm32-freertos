#ifndef CALLBACK_DRIVER_HPP
#define CALLBACK_DRIVER_HPP

#include "stm32hal.h"  // Include your HAL header that defines UART_HandleTypeDef, HAL_StatusTypeDef, etc.

// Generic CRTP Callback Driver Template
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
    // Constructor accepts the driver handle.
    explicit CallbackDriver(HandleType* handle)
        : handle_(handle) {}

    // Registers a callback.
    HAL_StatusTypeDef registerCallback(CBIDType cbId, CallbackType callback) {
        return RegFunc(handle_, cbId, callback);
    }

    // Unregisters a callback.
    HAL_StatusTypeDef unregisterCallback(CBIDType cbId) {
        return UnregFunc(handle_, cbId);
    }

protected:
    HandleType* handle_;  // Underlying driver handle.
};

#endif // CALLBACK_DRIVER_HPP
