// cli_commands.hpp
#pragma once

#include "cli_registry.hpp"
#include "fdcan_task.hpp"
#include <string>
#include <string_view>
#include <sstream>

extern FDCANTask* fdcan1Task;
extern FDCANTask* fdcan2Task;

inline void registerDefaultCLICommands(CLIRegistry& registry) {
    registry.registerCommand("help", [&](std::string_view) {
        return "Available commands: help, echo, add, setled, tab, cansend, canstat\r\n";
    });

    registry.registerCommand("echo", [](std::string_view args) {
        return std::string(args) + "\r\n";
    });

    registry.registerCommand("add", [](std::string_view args) {
        int a = 0, b = 0;
        std::istringstream iss(std::string(args));
        if (iss >> a >> b) {
            return "Sum: " + std::to_string(a + b) + "\r\n";
        }
        return "Usage: add <a> <b>\r\n";
    });

    registry.registerCommand("setled", [](std::string_view args) {
        if (args == "1") {
            // HAL_GPIO_WritePin(...) ← customize for your board
            return "LED ON\r\n";
        } else if (args == "0") {
            return "LED OFF\r\n";
        }
        return "Usage: setled <0|1>\r\n";
    });

    registry.registerCommand("tab", [&](std::string_view prefix) {
        // For now return fixed list
        return "Matching commands: help echo add setled tab cansend canstat\r\n";
    });

    registry.registerCommand("cansend", [](std::string_view args) {
        std::istringstream iss(std::string(args));
        int bus, id, len;
        uint8_t data[8] = {};

        if (!(iss >> bus >> std::hex >> id >> std::dec >> len)) {
            return "Usage: cansend <bus 1|2> <id hex> <len 0-8> <data...>\r\n";
        }

        for (int i = 0; i < len && i < 8; ++i) {
            int byte;
            if (!(iss >> std::hex >> byte)) return "Invalid data byte\r\n";
            data[i] = static_cast<uint8_t>(byte);
        }

        auto* task = (bus == 1) ? fdcan1Task : (bus == 2) ? fdcan2Task : nullptr;
        if (!task) return "Invalid CAN bus\r\n";

        FDCAN_TxHeaderTypeDef header{};
        header.Identifier = id;
        header.IdType = FDCAN_STANDARD_ID;
        header.TxFrameType = FDCAN_DATA_FRAME;
        header.DataLength = len << 16;
        header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        header.MessageMarker = 0;

        if (HAL_FDCAN_AddMessageToTxFifoQ(task->getHandle(), &header, data) != HAL_OK) {
            return "Failed to send CAN message\r\n";
        }

        return "CAN" + std::to_string(bus) + " Tx OK: ID=0x" + std::to_string(id) + "\r\n";
    });

    registry.registerCommand("canstat", [](std::string_view args) {
        auto* task = (args == "2") ? fdcan2Task : fdcan1Task;
        if (!task) return "Invalid CAN bus\r\n";

        const auto* h = task->getHandle();
        uint32_t status = h->Instance->PSR;
        std::ostringstream oss;
        oss << "FDCAN PSR: 0x" << std::hex << status << "\r\n";
        oss << "LEC: " << ((status >> 0) & 0x7);
        oss << ", ACT: " << ((status >> 3) & 0x3);
        oss << ", EP: " << ((status >> 5) & 0x3);
        oss << ", BO: " << ((status >> 7) & 0x1) << "\r\n";
        return oss.str();
    });
}
