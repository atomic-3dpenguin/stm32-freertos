/**
 * @file PrintfLogger.hpp
 * @brief A zero-dependency, header-only printf-style logger for embedded.
 *
 *  - txFunc is any function matching: void(const char* data, size_t len)
 *  - BufferSize must be between 1 and 1024 bytes
 *  - No dynamic allocation, no hidden base classes
 */
#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <type_traits>

namespace Jidoka {

    /// Prototype for your transmit callback
    using TransmitFunction = void (*)(const char* data, size_t len);

    template<TransmitFunction txFunc, std::size_t BufferSize = 256>
    class PrintfLogger {
        static_assert(BufferSize > 0 && BufferSize <= 1024,
                      "BufferSize must be in [1..1024]");

    public:
        PrintfLogger() noexcept = default;
        ~PrintfLogger() noexcept = default;

        // non-copyable, non-movable
        PrintfLogger(const PrintfLogger&) = delete;
        PrintfLogger& operator=(const PrintfLogger&) = delete;

        /** 
         * @brief Printf‐style log.  Builds a va_list and forwards to vLog.
         * @note noexcept: we swallow any vsnprintf errors.
         */
        void log(const char* format, ...) noexcept {
            va_list args;
            va_start(args, format);
            vLog(format, args);
            va_end(args);
        }

        /** 
         * @brief vPrintf‐style entry point.  Formats into a stack buffer
         *        and then calls txFunc(buffer, length).
         */
        void vLog(const char* format, va_list args) noexcept {
            char buffer[BufferSize];

            // vsnprintf returns # of chars that *would* have been written
            int ret = std::vsnprintf(buffer, BufferSize, format, args);
            if (ret < 0) {
                // encoding error: nothing to send
                return;
            }

            // truncate if necessary, guarantee null-termination
            std::size_t len = static_cast<std::size_t>(ret);
            if (len >= BufferSize) {
                len = BufferSize - 1;
                buffer[len] = '\0';
            }

            // hand off to your queue‐based txFunc
            txFunc(buffer, len);
        }
    };

} // namespace Jidoka

// // suppose you have:
// //   extern void enqueueToMyStaticLogQueue(const char*, size_t);
// // then you do:

// using MyLogger = Jidoka::PrintfLogger<
//     enqueueToMyStaticLogQueue,
//     512     // a bigger buffer if you like
// >;

// static MyLogger logger;

// // and in your macros:
// #define LOG_INFO(fmt, ...) \
//     logger.log("INFO [%s:%d] - " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
