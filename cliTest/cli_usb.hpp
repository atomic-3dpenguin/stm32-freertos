/*
 * cli_usb.hpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

#ifndef CLI_USB_HPP_
#define CLI_USB_HPP_
#include "cli_base.hpp"
#include "usbd_cdc_if.h"

namespace cli::transport {

class CLIUSB : public cli::CLIBase<CLIUSB> {
public:
    CLIUSB(const std::array<osEventFlagsId_t,8>& evts)
      : CLIBase(evts) {
        rxQ_ = osMessageQueueNew(64,sizeof(uint8_t),nullptr);
        CDC_Receive_FS(rxBuf_,sizeof(rxBuf_));
    }

    char getCharImpl() {
        uint8_t c;
        osMessageQueueGet(rxQ_,&c,nullptr,osWaitForever);
        return c;
    }
    void putCharImpl(char c) {
        uint8_t d=c;
        while(CDC_Transmit_FS(&d,1)==USBD_BUSY) osDelay(1);
    }

    void rxCallback(uint8_t* b,uint32_t l) {
        for(uint32_t i=0;i<l;++i)
            osMessageQueuePut(rxQ_,&b[i],0,0);
        CDC_Receive_FS(rxBuf_,sizeof(rxBuf_));
    }

private:
    osMessageQueueId_t rxQ_;
    uint8_t rxBuf_[64];
};

} // namespace cli::transport
#endif /* CLI_USB_HPP_ */
