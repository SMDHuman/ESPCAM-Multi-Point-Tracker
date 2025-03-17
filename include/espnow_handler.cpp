//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnow_handler.h"
#include "esp_now.h"
#include "WiFi.h"
#include "serial_com.h"
#include <Arduino.h>
#include "tracker.h"

#define MAX_PEERS 5

ESPNOW_MODES mode;
const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t device_mac[6] = {0}; 
uint8_t peer_list[MAX_PEERS][6] = {0};
size_t numof_peers = 0;

//-----------------------------------------------------------------------------
void espnow_init(){
  //...
  WiFi.macAddress(device_mac);
  //...
  if(esp_now_init() != ESP_OK){
    serial_send_slip("DEBUG");
    serial_send_slip(0);
    serial_send_slip("Error initializing ESP-NOW");
    serial_end_slip();
  }
  //...
  //esp_now_register_recv_cb(espnow_recv_cb);
  //...
  //uint8_t res = espnow_find_host();
}
//-----------------------------------------------------------------------------
void espnow_task(){

}