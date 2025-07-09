#ifndef SPINLESS_LOCK_HPP
#define SPINLESS_LOCK_HPP

/**
 * @file spinless_lock.hpp
 * @brief Header-only spinless lock for FreeRTOS that can be released
 *        from both task and ISR contexts via a single API using C++ RAII.
 *
 * Usage:
 *   os::freertos::SpinlessLock myLock;
 *   // ... before scheduler start or in application init
 *   // lock-init is automatic in constructor
 *
 *   // In task or ISR contexts:
 *   {
 *       os::freertos::SpinlessLockGuard guard(myLock);
 *       // critical section
 *   } // Guard destructor releases lock (ISR-aware)
 *
 * Internally uses a FreeRTOS binary semaphore and detects ISR context
 * to call the correct FreeRTOS give API.
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"  // for portYIELD_FROM_ISR

namespace os::freertos {

class SpinlessLock {
public:
    SpinlessLock() {
        handle = xSemaphoreCreateBinary();
        configASSERT(handle);
        // Prime the semaphore so first take succeeds
        BaseType_t hpw = pdFALSE;
        xSemaphoreGive(handle);
    }

    ~SpinlessLock() {
        vSemaphoreDelete(handle);
    }

    // non-copyable, non-movable
    SpinlessLock(const SpinlessLock&) = delete;
    SpinlessLock& operator=(const SpinlessLock&) = delete;

    /**
     * Acquire the lock (blocking). Must be called from a task.
     */
    void lock() {
        BaseType_t res = xSemaphoreTake(handle, portMAX_DELAY);
        configASSERT(res == pdTRUE);
    }

    /**
     * Release the lock. Can be called from task or ISR.
     */
    void unlock() {
        // Detect ISR context
        extern BaseType_t xPortIsInsideInterrupt(void);
        if (xPortIsInsideInterrupt() != pdFALSE) {
            BaseType_t hpw = pdFALSE;
            xSemaphoreGiveFromISR(handle, &hpw);
            portYIELD_FROM_ISR(hpw);
        } else {
            xSemaphoreGive(handle);
        }
    }

private:
    SemaphoreHandle_t handle;
};

/**
 * A simple RAII guard for SpinlessLock
 */
class SpinlessLockGuard {
public:
    explicit SpinlessLockGuard(SpinlessLock& lk) : lockRef(lk) {
        lockRef.lock();
    }
    ~SpinlessLockGuard() {
        lockRef.unlock();
    }
    SpinlessLockGuard(const SpinlessLockGuard&) = delete;
    SpinlessLockGuard& operator=(const SpinlessLockGuard&) = delete;

private:
    SpinlessLock& lockRef;
};

} // namespace os::freertos

#endif // SPINLESS_LOCK_HPP
