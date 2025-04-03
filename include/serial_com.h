//-----------------------------------------------------------------------------
// File: serial_com.h
// Last edit: 06/03/2025
//-----------------------------------------------------------------------------
#ifndef SERIAL_COM_H
#define SERIAL_COM_H
#include <Arduino.h>
#include "config_handler.h"

//-----------------------------------------------------------------------------
#define BAUDRATE CONFIGS.getInt("serial_baudrate", 115200)

//-----------------------------------------------------------------------------
void serial_init();
void serial_task(void * pvParameters);
void serial_send_slip(uint8_t* buf, size_t len);
void serial_send_slip(uint8_t data);
void serial_send_slip(uint8_t *data, uint32_t len);
void serial_send_slip(uint data);
void serial_send_slip(int data);
void serial_send_slip(char data);
void serial_send_slip(String data);
void serial_end_slip();

//-----------------------------------------------------------------------------
#endif
