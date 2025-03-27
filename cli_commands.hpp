// cli_commands.hpp
#pragma once

#include "cli_command_base.hpp"
#include <cstdio>
#include <cstring>

#include "logger_task.hpp"
#include "fdcan_tasks.hpp"

extern LoggerTask* logger;
extern FDCANTask* fdcan1Task;
extern FDCANTask* fdcan2Task;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

struct HelpCommand : CLICommand<HelpCommand> {
    static void execute(const char*, char* response, size_t maxLen) {
        snprintf(response, maxLen,
                 "Available commands:\r\n"
                 "help - Show this message\r\n"
                 "echo <text> - Echo back text\r\n"
                 "add <a> <b> - Add two integers\r\n"
                 "setled <0|1> - Toggle LED off/on\r\n"
                 "tab <prefix> - Autocomplete a command\r\n"
                 "cansend <bus> <id> <len> <data...> - Send CAN message\r\n"
                 "canstat <bus> - Show CAN status\r\n");
    }
};

struct TabCommand : CLICommand<TabCommand> {
    static const char* commands[];

    static void execute(const char* args, char* response, size_t maxLen) {
        size_t len = strlen(args);
        response[0] = '\0';

        for (int i = 0; commands[i]; ++i) {
            if (strncmp(commands[i], args, len) == 0) {
                strncat(response, commands[i], maxLen - strlen(response) - 1);
                strncat(response, "\r\n", maxLen - strlen(response) - 1);
            }
        }

        if (response[0] == '\0') {
            snprintf(response, maxLen, "No match for prefix: %s\r\n", args);
        }
    }
};

inline const char* TabCommand::commands[] = {
    "help",
    "echo",
    "add",
    "setled",
    "tab",
    "cansend",
    "canstat",
    nullptr
};

struct CanSendCommand : CLICommand<CanSendCommand> {
    static void execute(const char* args, char* response, size_t maxLen) {
        int bus = 0, id = 0, len = 0;
        uint8_t data[8] = {};

        if (sscanf(args, "%d %x %d %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx", &bus, &id, &len,
                   &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7]) >= 3) {
            if (len > 8) len = 8;

            FDCAN_TxHeaderTypeDef header = {
                .Identifier = (uint32_t)id,
                .IdType = FDCAN_STANDARD_ID,
                .TxFrameType = FDCAN_DATA_FRAME,
                .DataLength = (len << 16),
                .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
                .BitRateSwitch = FDCAN_BRS_OFF,
                .FDFormat = FDCAN_CLASSIC_CAN,
                .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
                .MessageMarker = 0
            };

            FDCAN_HandleTypeDef* hcan = (bus == 1) ? &hfdcan1 : &hfdcan2;
            if (HAL_FDCAN_AddMessageToTxFifoQ(hcan, &header, data) == HAL_OK) {
                snprintf(response, maxLen, "CAN%d Tx: ID=0x%X, len=%d\r\n", bus, id, len);
                if (logger) logger->log(LoggerTask::LOG_INFO, "CLI TX [CAN%d] ID:0x%X LEN:%d", bus, id, len);
            } else {
                snprintf(response, maxLen, "Failed to send CAN%d message\r\n", bus);
                if (logger) logger->log(LoggerTask::LOG_ERROR, "Failed to TX CAN%d ID:0x%X", bus, id);
            }
        } else {
            snprintf(response, maxLen, "Usage: cansend <bus 1|2> <id hex> <len 0-8> <data...>\r\n");
        }
    }
};

struct CanStatCommand : CLICommand<CanStatCommand> {
    static void execute(const char* args, char* response, size_t maxLen) {
        FDCAN_HandleTypeDef* hcan = (strcmp(args, "2") == 0) ? &hfdcan2 : &hfdcan1;

        uint32_t status = hcan->Instance->PSR;
        snprintf(response, maxLen,
                 "FDCAN PSR: 0x%08lX\r\n"  // Protocol Status Reg
                 "LEC: %lu, ACT: %lu, EP: %lu, BO: %lu\r\n",
                 status,
                 (status >> 0) & 0x7,  // LEC
                 (status >> 3) & 0x3,  // ACT
                 (status >> 5) & 0x3,  // EP
                 (status >> 7) & 0x1); // BO
    }
};