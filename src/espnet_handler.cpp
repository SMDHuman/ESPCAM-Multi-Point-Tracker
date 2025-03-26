//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnet_handler.h"
#include <Arduino.h>
#include "serial_com.h"
#include "esp_now.h"
#include "WiFi.h"
#include "tracker.h"

#define LED_BUILTIN 33

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len);

const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
espnet_config_t espnet_config;
espnet_config_t peer_list[MAX_PEERS];
uint8_t numof_peers;
uint8_t device_id;
esp_now_peer_info_t peer_info;

//-----------------------------------------------------------------------------
void espnet_init(){
  //...
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);
  //...
  WiFi.mode(WIFI_MODE_STA);
  if(esp_now_init() != ESP_OK){
    while(true){
      delay(100);
    }
  }
  //...
  WiFi.macAddress(espnet_config.mac);
  //...
  esp_now_register_recv_cb(espnow_recv_cb);
  //...
  memcpy(peer_info.peer_addr, broadcast_mac, 6);
  peer_info.channel = 0;
  peer_info.encrypt = false;
  esp_now_add_peer(&peer_info);
  //...
  espnet_config.mode = MODE_SEARCHING;
}
//-----------------------------------------------------------------------------
void espnet_task(){
  static uint32_t last_search = millis();
  static int32_t search_start = -1;
  // Searching for host
  if(espnet_config.mode == MODE_SEARCHING){
    if(millis() - last_search > 500){
      if(search_start == -1){
        search_start = millis();
      }
      if((millis() - search_start > ESPNET_TIMEOUT)){
        espnet_config.mode = MODE_HOST;
        for(uint8_t i = 0; i < 3; i++){
          digitalWrite(LED_BUILTIN, !HIGH);
          delay(100);
          digitalWrite(LED_BUILTIN, !LOW);
          delay(100);
        }
        digitalWrite(LED_BUILTIN, !HIGH);
      }
      else{
        uint8_t packet[] = {PACKET_REQ_JOIN};
        esp_now_send(broadcast_mac, packet, sizeof(packet));
      }
      last_search = millis();
    }
  }
  delay(1);
}

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len){
  ESPNET_PACKETS tag = (ESPNET_PACKETS)data[0];
  data = &data[1];
  len -= 1;
  switch (tag){
    //-------------------------------------------------------------------------
    case PACKET_REQ_PING:
    {
      uint8_t packet[] = {PACKET_RSP_PONG};
      esp_now_send(mac, packet, sizeof(packet));
    }break;
    case PACKET_REQ_LEDTOGGLE:
    {
      if(len > 0){
        digitalWrite(LED_BUILTIN, !data[0]);
      }
      else{
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_JOIN:
    {
      if(espnet_config.mode == MODE_HOST){
        if(numof_peers < MAX_PEERS){
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
    //-------------------------------------------------------------------------
    case PACKET_RSP_JOIN:
    {
      if(data[0] > 0){
        espnet_config.id = data[0];
        espnet_config.mode = MODE_CLIENT;
        memcpy(espnet_config.host_mac, mac, 6);
        // add peer
        memcpy(peer_info.peer_addr, mac, 6);
        peer_info.channel = 0;
        peer_info.encrypt = false;
        esp_now_add_peer(&peer_info);
      }
      else{
        espnet_config.mode = MODE_NONE;
      }
    }break;
    //-------------------------------------------------------------------------
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
      if(espnet_config.mode == MODE_CLIENT){
        espnet_config.mode = MODE_NONE;
        memset(espnet_config.host_mac, 0, 6);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_POINTS:
    {
      uint8_t *packet = (uint8_t*)malloc(sizeof(point_rect_t)*tracker_points_len+1);
      packet[0] = PACKET_RSP_POINTS;
      memcpy(packet+1, tracker_points_rect, sizeof(point_rect_t)*tracker_points_len);
      esp_now_send(mac, packet, sizeof(point_rect_t)*tracker_points_len+1);
      free(packet);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_FCOUNT:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_CONFIG:
    {
      uint8_t *packet = (uint8_t*)malloc(sizeof(espnet_config)+1);
      packet[0] = PACKET_RSP_CONFIG;
      memcpy(packet+1, &espnet_config, sizeof(espnet_config));
      esp_now_send(mac, packet, sizeof(espnet_config)+1);
      free(packet);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_SET_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    //-------------------------------------------------------------------------
    default:
    {

    }break;
  }
}