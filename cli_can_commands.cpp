// cli_can_commands.hpp
#pragma once

#include "cli_command_base.hpp"
#include "fdcan_tasks.hpp"
#include <cstdio>
#include <cstring>

extern FDCANTask* fdcan1Task;
extern FDCANTask* fdcan2Task;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

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
            } else {
                snprintf(response, maxLen, "Failed to send CAN%d message\r\n", bus);
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
