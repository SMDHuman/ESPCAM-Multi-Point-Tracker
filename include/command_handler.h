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
};

enum RESP_TYPE_E{
    RESP_NONE = 0,
    RESP_PONG,
    RESP_FRAME,
    RESP_POINTS,
    RESP_FCOUNT,
    RESP_CONFIG,
    RESP_ERROR,
};

//-----------------------------------------------------------------------------
void CMD_init();
void CMD_task();
void CMD_parse(uint8_t *msg_data, uint32_t package_size);

#endif