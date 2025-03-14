//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnow_handler.h"
#include "esp_now.h"
#include "WiFi.h"
#include "serial_com.h"
#include <Arduino.h>

#define MAX_PEERS 5

ESPNOW_MODES mode;
const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t device_mac[6] = {0}; 

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
  esp_now_register_recv_cb(espnow_recv_cb);
  //...
  esp_now_peer_info_t peer_info;
  memcpy(peer_info.peer_addr, broadcast_mac, 6);
  peer_info.channel = 0;
  peer_info.encrypt = false;
  esp_now_add_peer(&peer_info);
}
//-----------------------------------------------------------------------------
void espnow_task(){

}
//-----------------------------------------------------------------------------
uint8_t espnow_find_host(int32_t timeout = 1000){
  int32_t start = millis();
  uint8_t peer_addr[6];
  memcpy(peer_addr, broadcast_mac, 6);
  while((millis() - start < timeout) & mode == MODE_NONE){
    espnow_send(peer_addr, TAG_JOIN, 0, 0);
    delay(100);
  }
  if(mode == MODE_NONE){
    espnow_init_host();
  }
}
//-----------------------------------------------------------------------------
uint8_t espnow_init_host(){
  mode = MODE_HOST;
}
//-----------------------------------------------------------------------------
void espnow_send(uint8_t *peer_addr, uint8_t tag, uint8_t *data, uint8_t len){
  uint8_t *msg = (uint8_t *)malloc(len + 1);
  msg[0] = tag;
  memcpy(msg + 1, data, len);
  esp_now_send(peer_addr, msg, len + 1);
  free(msg);
}
//-----------------------------------------------------------------------------
static void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len){
  uint8_t tag = data[0];
  data = data + 1;
  data_len--;
  uint8_t addr[6];
  memcpy(addr, mac_addr, 6);
  switch(tag){
    case TAG_JOIN:
      if(mode == MODE_HOST){
        esp_now_peer_num_t num;
        esp_now_get_peer_num(&num);
        if(num.total_num < MAX_PEERS){
          espnow_send(addr, TAG_REJECT, 0, 0);
          break;
        }
        esp_now_peer_info_t peer_info;
        memcpy(peer_info.peer_addr, addr, 6);
        peer_info.channel = 0;
        peer_info.encrypt = false;
        esp_now_add_peer(&peer_info);
        espnow_send(addr, TAG_ACCEPT, 0, 0);
      }
      break;
    case TAG_ACCEPT:
      if(mode == MODE_NONE){
        mode = MODE_CLIENT;
      }
      break;
    case TAG_REJECT:
      if(mode == MODE_NONE){
        mode = MODE_REJECTED;
      }
      break;
  }
}
