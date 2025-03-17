//-----------------------------------------------------------------------------
// File: command_handler.cpp
// Last modified: 12/03/2025
//-----------------------------------------------------------------------------
#include "command_handler.h"
#include <Arduino.h>
#include "serial_com.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void CMD_init(){
}
//-----------------------------------------------------------------------------
void CMD_task(){
}

//-----------------------------------------------------------------------------
void CMD_parse(uint8_t *msg_data, uint32_t len){
  static uint8_t *peer_addr;
  static esp_err_t res = ESP_OK;
  CMD_TYPE_E cmd = (CMD_TYPE_E)msg_data[0];
  // Reuse the global peer_info instead of creating a local one
  switch(cmd){

  }
}
