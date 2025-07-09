/*
 * myThread.hpp
 *
 *  Created on: May 18, 2025
 *      Author: cavem
 */

#ifndef MYTHREAD_HPP_
#define MYTHREAD_HPP_

#include "main.h"

#include "CrtpThread.hpp"
#include "cmsis_os2.h"
#include <cstdio>

namespace common{
class MyThread : public CrtpThread<MyThread> {
public:
  // Automatically starts with high priority, 1 KiB stack, named "MyThr"
  MyThread()
    : CrtpThread(osPriorityHigh, 1024, "MyThr")
  {}

  // This is the entry point for the new thread
  void run() {
    constexpr uint32_t FLAG_WORK = (1u << 0);

    for (;;) {
        HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);
      // Wait for FLAG_WORK from either thread or ISR
      uint32_t ev = waitEvent(FLAG_WORK);

      if (ev & FLAG_WORK) {
        // ... do your work here ...
    	  printf("Working\r\n");
      }
    }
  }

  // Can be called from thread context
  void triggerWork() {
    sendEvent(1u << 0);
  }

  // Can be called from an ISR too
  void triggerWorkFromISR() {
    sendEvent(1u << 0);
  }
};
}





#endif /* MYTHREAD_HPP_ */
