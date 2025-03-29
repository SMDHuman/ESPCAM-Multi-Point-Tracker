//-----------------------------------------------------------------------------
// File: command_handler.h
// Last edit: 06/03/2025
//-----------------------------------------------------------------------------
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H
#include <Arduino.h>

//-----------------------------------------------------------------------------
enum CMD_TYPE_E{
    CMD_NONE = 0,
    CMD_PING,
    CMD_RQ_FRAME,
    CMD_RQ_POINTS,
    CMD_RQ_FCOUNT,
    CMD_RQ_CONFIG,
    CMD_SET_CONFIG,
    CMD_RQ_PEERLIST,
    CMD_RQ_PEERCOUNT,
};

enum RSP_TYPE_E{
    RSP_NONE = 0,
    RSP_PONG,
    RSP_FRAME,
    RSP_POINTS,
    RSP_FCOUNT,
    RSP_CONFIG,
    RSP_ERROR,
    RSP_ESPNET_ERROR,
    RSP_PEERLIST,
    RSP_PEERCOUNT,
};

//-----------------------------------------------------------------------------
void CMD_init();
void CMD_task();
void CMD_parse(uint8_t *msg_data, uint32_t package_size);

#endif