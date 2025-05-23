// Log.hpp
#ifndef JIDOKA_LOG_HPP
#define JIDOKA_LOG_HPP

#include "Logger.hpp"        // your PrintfLogger<>
#include "FreeRTOS.h"        // for xPortIsInsideInterrupt()
#include <chrono>
#include <cstddef>
#include <cstdio>

//-----------------------------------------------------------------------------
//  STEP 1: You Must Define Your Transmit Callback
//-----------------------------------------------------------------------------
// A function matching: void fn(const char* data, size_t len);
// e.g.
//   void myUartTx(const char* data, size_t len) { … }
// before you do:  #include "Log.hpp"
#ifndef LOG_TX_CALLBACK
#  error "Please #define LOG_TX_CALLBACK(yourTransmitFunction) before including Log.hpp"
#endif

//-----------------------------------------------------------------------------
//  STEP 2: In the same TU (or a common header), do:
//    #define LOG_TX_CALLBACK(myUartTx)
//    #include "Log.hpp"
//-----------------------------------------------------------------------------

namespace Jidoka {

    // clock alias
    using Clock = std::chrono::steady_clock;

    // capture the moment we first include this header
    static inline const Clock::time_point start_time = Clock::now();

    // format into "hh:mm:ss:ms" (13 bytes including '\0')
    static inline void getTimeString(char* buf, std::size_t bufLen) {
        auto elapsed = Clock::now() - start_time;
        auto msTotal = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        auto hrs  = (msTotal / 3'600'000) % 24;
        auto mins = (msTotal /    60'000) % 60;
        auto secs = (msTotal /     1'000) % 60;
        auto ms   =  msTotal % 1'000;

        std::snprintf(buf, bufLen,
                      "%02lld:%02lld:%02lld:%03lld",
                      static_cast<long long>(hrs),
                      static_cast<long long>(mins),
                      static_cast<long long>(secs),
                      static_cast<long long>(ms));
    }

    // your single, inline logger instance, using *your* callback:
    inline PrintfLogger<LOG_TX_CALLBACK> logger;

} // namespace Jidoka

//-----------------------------------------------------------------------------
//  The four LOG_ macros
//-----------------------------------------------------------------------------
#define LOG_DEBUG(fmt, ...)                                                      \
    do {                                                                          \
        char _tbuf[13];                                                           \
        Jidoka::getTimeString(_tbuf, sizeof(_tbuf));                              \
        Jidoka::logger.log(                                                      \
            "DEBUG [%s:%d] [%s] - " fmt "\n",                                     \
            __FUNCTION__, __LINE__, _tbuf, ##__VA_ARGS__);                        \
    } while (0)

#define LOG_INFO(fmt, ...)                                                       \
    do {                                                                          \
        char _tbuf[13];                                                           \
        Jidoka::getTimeString(_tbuf, sizeof(_tbuf));                              \
        Jidoka::logger.log(                                                      \
            "INFO  [%s:%d] [%s] - " fmt "\n",                                     \
            __FUNCTION__, __LINE__, _tbuf, ##__VA_ARGS__);                        \
    } while (0)

#define LOG_WARN(fmt, ...)                                                       \
    do {                                                                          \
        char _tbuf[13];                                                           \
        Jidoka::getTimeString(_tbuf, sizeof(_tbuf));                              \
        Jidoka::logger.log(                                                      \
            "WARN  [%s:%d] [%s] - " fmt "\n",                                     \
            __FUNCTION__, __LINE__, _tbuf, ##__VA_ARGS__);                        \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                      \
    do {                                                                          \
        char _tbuf[13];                                                           \
        Jidoka::getTimeString(_tbuf, sizeof(_tbuf));                              \
        Jidoka::logger.log(                                                      \
            "ERROR [%s:%d] [%s] - " fmt "\n",                                     \
            __FUNCTION__, __LINE__, _tbuf, ##__VA_ARGS__);                        \
    } while (0)

#endif // JIDOKA_LOG_HPP
