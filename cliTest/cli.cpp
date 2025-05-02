/*
 * cli.cpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

// cli.cpp
#include "cli_usb.hpp"
#include "cli_common.hpp"
#include "cmsis_os2.h"
#include "usbd_cdc_if.h"

using namespace cli;
using namespace cli::transport;

// Global pointer for USB callback
CLIUSB* gCliUsb = nullptr;

// USB CDC Rx complete callback (called from USBD interrupt)
int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t Len) {
    if (gCliUsb) {
        gCliUsb->rxCallback(Buf, Len);
    }
    return USBD_OK;
}

// FreeRTOS thread entry for CLI over USB-VCOM
void USBTask(void* argument) {
    // Create one event flag group per possible subsystem
    std::array<osEventFlagsId_t,8> evts;
    for (auto& e : evts) {
        e = osEventFlagsNew(nullptr);
    }

    // Instantiate CLIUSB and assign global pointer
    CLIUSB cli(evts);
    gCliUsb = &cli;

    // Register 'help' command
    cli.registerCommand(makeCmd(Subsystem::CORE, CoreCmd::Help), "help", [&](auto& args){
        cli.listCommands();
    });

    // Register 'ledON' command: ON|OFF, bank 1-6, R:0xNN, G:0xNN, B:0xNN
    cli.registerCommand(makeCmd(Subsystem::LED, LedCmd::RGB), "ledON", [&](auto& args){
        if (args.size() < 6) {
            cli.write("Usage: ledON <ON|OFF> <1-6> <R:0xNN> <G:0xNN> <B:0xNN>\r\n");
            return;
        }
        bool    on   = (args[1] == "ON");
        int     bank = std::stoi(args[2]);
        uint8_t r    = static_cast<uint8_t>(std::stoul(args[3].substr(2), nullptr, 16));
        uint8_t g    = static_cast<uint8_t>(std::stoul(args[4].substr(2), nullptr, 16));
        uint8_t b    = static_cast<uint8_t>(std::stoul(args[5].substr(2), nullptr, 16));
        // TODO: call your hardware-level RGB set function:
        // setRGB(bank, on ? r : 0, on ? g : 0, on ? b : 0);
    });

    // Run the CLI (blocking)
    cli.run();
}
