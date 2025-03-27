//-----------------------------------------------------------------------------
// File: command_handler.cpp
// Last modified: 12/03/2025
//-----------------------------------------------------------------------------
#include "command_handler.h"
#include <Arduino.h>
#include "serial_com.h"
#include "tracker.h"

//-----------------------------------------------------------------------------
void CMD_init(){
}

//-----------------------------------------------------------------------------
void CMD_task(){
}

//-----------------------------------------------------------------------------
void CMD_parse(uint8_t *msg_data, uint32_t len){
  CMD_TYPE_E cmd = (CMD_TYPE_E)msg_data[0];
  uint8_t *data = msg_data + 1; 
  esp_err_t res;
  switch(cmd){
  }
}
