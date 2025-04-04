//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnet.h"
#include <Arduino.h>
#include "serial_com.h"
#include "esp_now.h"
#include "WiFi.h"
#include "tracker.h"
#include "command_handler.h"

#define P_ARRAY_IMPLEMENTATION
#include "p_array.h"

//-----------------------------------------------------------------------------
void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len);

const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
espnet_config_t espnet_config;
array *peer_list;

static esp_now_peer_info_t peer_info;
static ESPNET_MODES static_mode = MODE_NONE;

//-----------------------------------------------------------------------------
void espnet_init(){
  //...
  peer_list = array_create(MAX_PEERS, sizeof(espnet_config_t));
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
  if(static_mode == MODE_HOST){
    espnet_config.mode = MODE_HOST;
  }
  else{
    espnet_config.mode = MODE_SEARCHING;
  }
  if(static_mode == MODE_SEARCHING){
    static_mode = MODE_NONE;
  }
}

//-----------------------------------------------------------------------------
void espnet_task(void * pvParameters ){
  static uint32_t last_search = millis();
  static int32_t search_start = -1;
  while(1){
    // Searching for host
    if((espnet_config.mode == MODE_SEARCHING)){
      if(millis() - last_search > ESPNET_SEARCH_INTERVAL){
        if(search_start == -1){
          search_start = millis();
        }
        if((millis() - search_start > ESPNET_TIMEOUT_SEARCH)){
          if(static_mode == MODE_CLIENT){
            search_start = -1;
          }else{
            espnet_config.mode = MODE_HOST;
            for(uint8_t i = 0; i < 3; i++){
              digitalWrite(LED_BUILTIN, !HIGH);
              delay(100);
              digitalWrite(LED_BUILTIN, !LOW);
              delay(100);
            }
            digitalWrite(LED_BUILTIN, !HIGH);
          }
        }
        else{
          uint8_t packet[2] = {PACKET_REQ_JOIN, 0};
          esp_now_send(broadcast_mac, packet, 2);
        }
        last_search = millis();
      }
    }

    // Check for lost connections
    if((espnet_config.mode == MODE_HOST) || (espnet_config.mode == MODE_CLIENT)){
      for(uint8_t i = 0; i < peer_list->length; i++){
        // Remove peers that haven't responded for a while
        espnet_config_t peer_config;
        array_get(peer_list, i, &peer_config);
        if(millis() - peer_config.last_response > ESPNET_TIMEOUT_CONLOST){
          esp_now_del_peer(peer_config.mac);
          array_remove(peer_list, i);
        }
        // Send ping to peers that haven't responded for a while
        else if((millis() - peer_config.last_response > ESPNET_TIMEOUT_PING) && (espnet_config.mode == MODE_HOST)){
          espnet_send(PACKET_REQ_PING, peer_config.id);
        }
      }
    }

    vTaskDelay(1);
  }
}
//-----------------------------------------------------------------------------
// Load configs for ESP-NOW
void espnet_load_configs(){
  static_mode = (ESPNET_MODES)CONFIGS.getInt("espnet_mode", MODE_NONE);
}

//-----------------------------------------------------------------------------
// Send data to a peer
void espnet_send(ESPNET_PACKETS tag, uint8_t id){
  espnet_send(tag, id, {}, 0);
}
// Send data to a peer
void espnet_send(ESPNET_PACKETS tag, uint8_t id, uint8_t *data, uint32_t len){
  if(len > 250) return;
  for(uint8_t i = 0; i < peer_list->length; i++){
    espnet_config_t peer_config;
    array_get(peer_list, i, &peer_config);
    if(peer_config.id == id){
      uint8_t packet[2+len];
      packet[0] = tag;
      packet[1] = espnet_config.id;
      memcpy(packet+2, data, len);
      esp_err_t res = esp_now_send(peer_config.mac, packet, 2+len);
      //...
      //if(res != ESP_OK){
      //  serial_send_slip(CMD_RSP_ESPNET_ERROR);
      //  serial_send_slip(res);
      //  serial_end_slip();
      //} 
      return;
    }
  }
}

//-----------------------------------------------------------------------------
// Check if the id is already in the peer list
uint8_t espnet_check_id(uint8_t id){
  for(uint8_t i = 0; i < peer_list->length; i++){
    espnet_config_t peer_config;
    array_get(peer_list, i, &peer_config);
    if(peer_config.id == id){
      return 1;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
void espnow_recv_cb(const uint8_t *addr, const uint8_t *data, int len){
  // Check if the packet is for us
  if(len < 2) return;
  ESPNET_PACKETS tag = (ESPNET_PACKETS)data[0];
  uint8_t from_id = data[1];
  data += 2;
  len -= 2;
  // Update last response times
  for(uint8_t i = 0; i < peer_list->length; i++){
    espnet_config_t peer_config;
    array_get(peer_list, i, &peer_config);
    if(peer_config.id == from_id){
      peer_config.last_response = millis();
      array_set(peer_list, i, &peer_config);
      break;
    }
  }
  //Serial.printf("ESP-NET: %02X:%02X:%02X:%02X:%02X:%02X -> %d\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  //Serial.printf("ESP-NET: %d -> %d\n", from_id, tag);
  //...
  switch (tag){
    //-------------------------------------------------------------------------
    case PACKET_REQ_PING:
    {
      espnet_send(PACKET_RSP_PONG, from_id, {}, 0);
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
        if(peer_list->length < MAX_PEERS){
          //...
          memcpy(peer_info.peer_addr, addr, 6);
          peer_info.channel = 0;
          peer_info.encrypt = false;
          esp_now_add_peer(&peer_info);
          //...
          espnet_config_t peer_config;
          peer_config.id = peer_list->length+1;
          memcpy(peer_config.mac, addr, 6);
          peer_config.mode = MODE_CLIENT;
          peer_config.last_response = millis();
          array_push(peer_list, &peer_config);
          //...
          uint8_t packet[3] = {PACKET_RSP_JOIN_ACCEPT, espnet_config.id, peer_config.id};
          esp_now_send(addr, packet, 3);
        }
        else{
          uint8_t packet[2] = {PACKET_RSP_JOIN_DENY, espnet_config.id};
          esp_now_send(addr, packet, 2);
        }
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_JOIN_ACCEPT:
    {
      espnet_config.id = data[0];
      espnet_config.mode = MODE_CLIENT;
      // add peer
      memcpy(peer_info.peer_addr, addr, 6);
      peer_info.channel = 0;
      peer_info.encrypt = false;
      esp_now_add_peer(&peer_info);
      //...
      espnet_config_t peer_config;
      peer_config.id = 0;
      memcpy(peer_config.mac, addr, 6);
      peer_config.mode = MODE_CLIENT;
      peer_config.last_response = millis();
      array_push(peer_list, &peer_config);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_JOIN_DENY:
    {
      espnet_config.mode = MODE_NONE;
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_POINTS:
    {
      if(espnet_config.mode == MODE_CLIENT){
        espnet_send(PACKET_RSP_POINTS, from_id, (uint8_t*)tracker_points_rect, sizeof(point_rect_t)*tracker_points_len);  
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_POINTS:
    {
      uint8_t *packet = (uint8_t*)malloc(2+len);
      packet[0] = CMD_RSP_POINTS;
      packet[1] = from_id;
      memcpy(packet+2, data, len);
      serial_send_slip(packet, 2+len);
      serial_end_slip();
      free(packet);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_FCOUNT:
    {
      if(espnet_config.mode == MODE_CLIENT){
        espnet_send(PACKET_RSP_FCOUNT, from_id, (uint8_t*)&tracker_frame_count, 8);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_FCOUNT:
    {
      if(espnet_config.mode == MODE_HOST){
        uint8_t *packet = (uint8_t*)malloc(8);
        packet[0] = CMD_RSP_FCOUNT;
        packet[1] = from_id;
        memcpy(packet+2, data, 8);
        serial_send_slip(packet, 10);
        serial_end_slip();
        free(packet);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_ESPNET_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        espnet_send(PACKET_RSP_ESPNET_CONFIG, from_id, (uint8_t*)&espnet_config, sizeof(espnet_config));
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_SET_ESPNET_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        //...
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_CONFIG:
    {
      char req_key[16] = {0};
      memcpy(req_key, data, len);
      if(CONFIGS.isKey(req_key)){
        uint8_t *packet = (uint8_t*)malloc(len+4);
        uint32_t value = CONFIGS.getInt(req_key);
        memcpy(packet, req_key, len);
        memcpy(packet+len, &value, 4);
        espnet_send(PACKET_RSP_CONFIG, from_id, packet, len+4);
        free(packet);
      }else{
        espnet_send(PACKET_RSP_ERROR, from_id, (uint8_t*)req_key, len);
      }
    }
    //-------------------------------------------------------------------------
    case PACKET_RSP_CONFIG:
    {
      if(espnet_config.mode == MODE_HOST){
        serial_send_slip((uint8_t)CMD_RSP_CONFIG);
        serial_send_slip(from_id);
        if(len > 0){
          serial_send_slip((uint8_t*)data, len);
        }
        serial_end_slip();
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_SET_CONFIG:
    {
      char req_key[16] = {0};
      memcpy(req_key, data, len-4);
      uint32_t value = *(uint32_t*)(data+len-4);
      if(CONFIGS.isKey(req_key)){
        CONFIGS.putInt(req_key, value);
        espnet_send(PACKET_RSP_CONFIG, from_id);
      }else{
        espnet_send(PACKET_RSP_ERROR, from_id, (uint8_t*)req_key, len);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_RELOAD_CONFIG:
    {
      config_reload();
      espnet_send(PACKET_RSP_RELOAD_CONFIG, from_id);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_RELOAD_CONFIG:
    {
      serial_send_slip((uint8_t)CMD_RSP_RELOAD_CONFIG);
      serial_send_slip(from_id);
      serial_end_slip();
    }break;
    //-------------------------------------------------------------------------
    default:
    {

    }break;
  }
}
