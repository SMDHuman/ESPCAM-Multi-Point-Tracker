//-----------------------------------------------------------------------------
// File: command_handler.h
// Last edit: 06/03/2025
//-----------------------------------------------------------------------------
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H
#include <Arduino.h>

//-----------------------------------------------------------------------------
enum CMD_TYPE_E{
};

enum RESP_TYPE_E{
};

//-----------------------------------------------------------------------------
void CMD_init();
void CMD_task();
void CMD_parse(uint8_t *msg_data, uint32_t package_size);

#endif