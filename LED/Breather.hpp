// Breather.hpp
#pragma once
#include <cmath>
#include <cstdint>
#include "MockLEDDriver.hpp"

class Breather {
public:
    /// @param period_ms   full inhale+exhale time (ms)
    /// @param min_level   lowest PWM value (0–255)
    /// @param max_level   highest PWM value (0–255)
    Breather(uint32_t period_ms = 2000,
             uint8_t  min_level =  10,
             uint8_t  max_level = 200)
      : period_ms_(period_ms)
      , min_(min_level)
      , max_(max_level)
    {}

    /// Call this at least as often as your update interval.
    /// @param now_ms  absolute time in ms (e.g. ticks * portTICK_PERIOD_MS)
    void update(uint32_t now_ms) {
        // phase ∈ [0,1)
        float phase = float(now_ms % period_ms_) / float(period_ms_);
        // cosine‐based ease (starts at min, peaks at max at phase=0.5, back at min at phase=1)
        float curve = 0.5f * (1.0f - std::cosf(2.0f * 3.14159265f * phase));
        uint8_t level = min_ + uint8_t(curve * float(max_ - min_));
        MockLEDDriver::setBrightness(level);
    }

private:
    uint32_t period_ms_;
    uint8_t  min_, max_;
};


// // BreathingTask.cpp
// #include "FreeRTOS.h"
// #include "task.h"
// #include "Breather.hpp"

// // Task entry – create with xTaskCreate(vBreathingTask,…)
// extern "C" void vBreathingTask(void* /*pvParameters*/) {
//     constexpr uint32_t update_interval_ms = 10;
//     Breather breather(3000, 20, 230);  // 3 s breathe cycle, 20→230 PWM

//     TickType_t lastWake = xTaskGetTickCount();
//     for (;;) {
//         // convert ticks to ms
//         uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
//         breather.update(now_ms);

//         // wait exactly update_interval_ms before next update
//         vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(update_interval_ms));
//     }
// }
