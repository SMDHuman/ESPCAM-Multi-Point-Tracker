//-----------------------------------------------------------------------------
// File: command_handler.h
// Last edit: 06/03/2025
//-----------------------------------------------------------------------------
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H
#include <Arduino.h>

//-----------------------------------------------------------------------------
enum CMD_PACKET_TYPE_E{
    CMD_NONE = 0,
    CMD_RSP_ERROR,
    CMD_RSP_ESPNET_ERROR,
    CMD_REQ_PING,
    CMD_RSP_PONG,
    CMD_REQ_FRAME,
    CMD_RSP_FRAME,
    CMD_REQ_POINTS,
    CMD_RSP_POINTS,
    CMD_REQ_FCOUNT,
    CMD_RSP_FCOUNT,
    CMD_REQ_CONFIG,
    CMD_RSP_CONFIG,
    CMD_SET_CONFIG,
    CMD_REQ_PEERLIST,
    CMD_RSP_PEERLIST,
    CMD_REQ_PEERCOUNT,
    CMD_RSP_PEERCOUNT,
    CMD_REBOOT,
};

//-----------------------------------------------------------------------------
void command_init();
void command_task();
void command_parse(uint8_t *msg_data, uint32_t package_size);

#endif