//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnet_handler.h"
#include <Arduino.h>
#include "serial_com.h"

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len);

const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
espnet_config_t espnet_config;
espnet_config_t peer_list[MAX_PEERS];
uint8_t numof_peers;
uint8_t device_id;

//-----------------------------------------------------------------------------
void espnet_init(){
  //...
  WiFi.macAddress(espnet_config.mac);
  //...
  if(esp_now_init() != ESP_OK){
    serial_send_slip("DEBUG");
    serial_send_slip(0);
    serial_send_slip("Error initializing ESP-NOW");
    serial_end_slip();
  }
  //...
  esp_now_register_recv_cb(espnow_recv_cb);
  
  { // Find host and register yourserf as a client
    esp_now_peer_info_t peer_info;
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    esp_now_add_peer(&peer_info);
    //...
    uint32_t now = millis();
    uint8_t packet[] = {PACKET_REQ_JOIN};
    while(now - millis() < ESPNET_TIMEOUT){
      esp_now_send(broadcast_mac, packet, sizeof(packet));
      if(espnet_config.mode == MODE_CLIENT){
        break;
      }
      delay(100);
    }
    // Be a host if no host is found
    if(espnet_config.mode == MODE_NONE){
      espnet_config.mode = MODE_HOST;
    }
  }
}
//-----------------------------------------------------------------------------
void espnet_task(){

}

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len){
  ESPNET_PACKETS tag = (ESPNET_PACKETS)data[0];
  data += 1;
  len -= 1;
  switch (tag){
    case PACKET_REQ_PING:
    {
      uint8_t packet[] = {PACKET_RSP_PONG};
      esp_now_send(mac, packet, sizeof(packet));
    }break;
    case PACKET_REQ_JOIN:
    {
      if(espnet_config.mode == MODE_HOST){
        if(numof_peers < MAX_PEERS){
          esp_now_peer_info_t peer_info;
          memcpy(peer_info.peer_addr, mac, 6);
          peer_info.channel = 0;
          peer_info.encrypt = false;
          esp_now_add_peer(&peer_info);
          peer_list[numof_peers].id = numof_peers + 1;
          memcpy(peer_list[numof_peers].mac, mac, 6);
          peer_list[numof_peers].mode = MODE_CLIENT;
          numof_peers++;
          uint8_t packet[] = {PACKET_RSP_JOIN, peer_list[numof_peers].id};
          esp_now_send(mac, packet, sizeof(packet));
        }
        else{
          uint8_t packet[] = {PACKET_RSP_JOIN, 0};
          esp_now_send(mac, packet, sizeof(packet));
        }
      }
    }break;
    case PACKET_REQ_LEAVE:
    {
      if(espnet_config.mode == MODE_HOST){
        for(uint8_t i = 0; i < numof_peers; i++){
          if(memcmp(peer_list[i].mac, mac, 6) == 0){
            esp_now_del_peer(peer_list[i].mac);
            for(uint8_t j = i; j < numof_peers - 1; j++){
              peer_list[j] = peer_list[j + 1];
            }
            numof_peers--;
            break;
          }
        }
        uint8_t packet[] = {PACKET_RSP_LEAVE};
        esp_now_send(mac, packet, sizeof(packet));
      }
    }break;
    case PACKET_REQ_POINTS:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    case PACKET_REQ_FCOUNT:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    case PACKET_REQ_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    case PACKET_SET_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_JOIN:
    {
      if(data[0] != 0){
        device_id = data[0];
        espnet_config.mode = MODE_CLIENT;
        // add peer
        esp_now_peer_info_t peer_info;
        memcpy(peer_info.peer_addr, mac, 6);
        peer_info.channel = 0;
        peer_info.encrypt = false;
        esp_now_add_peer(&peer_info);
      }
    }
    default:
      break;
  }
}