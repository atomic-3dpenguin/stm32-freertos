#ifndef CRTPTHREAD_HPP_
#define CRTPTHREAD_HPP_

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <cstdio>

namespace common{

template<typename Derived>
class CrtpThread {
public:
  CrtpThread(osPriority_t priority = osPriorityNormal,
             uint32_t    stack_size = 512,
             const char* name       = nullptr)
  {
    // Create an event-flags object
    evt_id_ = osEventFlagsNew(nullptr);

    // Set up thread attributes
    osThreadAttr_t attr{};
    attr.name       = name;
    attr.priority   = priority;
    attr.stack_size = stack_size;

    // Spawn the thread, passing 'this' as argument
    thread_id_ = osThreadNew(&CrtpThread::threadEntry, this, &attr);
  }

  /// Send an event flag; picks the ISR-safe API if inside an interrupt.
  void sendEvent(uint32_t flags) {

      osEventFlagsSet(evt_id_, flags);
  }

  /// Wait until any of the specified flags are set (or timeout)
  uint32_t waitEvent(uint32_t flags, uint32_t timeout = osWaitForever) {
    // osFlagsWaitAny will clear the bits by default; adjust if you need different behavior
    return osEventFlagsWait(evt_id_, flags, osFlagsWaitAny, timeout);
  }

  /// Dump information for all tasks to the console
  static void Dump() {
    // Header for FreeRTOS vTaskList output
    printf("Name\t         State   Prio    Stack   Num\r\n");
#if (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
    char buffer[512];
    vTaskList(buffer);
    printf("%s\r\n", buffer);
#else
    printf("Error: configUSE_TRACE_FACILITY and configUSE_STATS_FORMATTING_FUNCTIONS must be enabled in FreeRTOSConfig.h\r\n");
#endif
  }
protected:

  /// Cast back to the derived class and call its run()
  static void threadEntry(void* arg) {
    static_cast<Derived*>(arg)->run();
  }

  osThreadId_t     thread_id_;
  osEventFlagsId_t evt_id_;
};

} // common
#endif /* CRTPTHREAD_HPP_ */
