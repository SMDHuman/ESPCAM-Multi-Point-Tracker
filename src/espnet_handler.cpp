//-----------------------------------------------------------------------------
// File: espnow_handler.cpp
//-----------------------------------------------------------------------------
#include "espnet_handler.h"
#include <Arduino.h>
#include "serial_com.h"
#include "esp_now.h"
#include "WiFi.h"
#include "tracker.h"
#include "command_handler.h"
#include "config_handler.h"

//-----------------------------------------------------------------------------
void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int len);

const uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
espnet_config_t espnet_config;
espnet_config_t peer_list[MAX_PEERS];
uint8_t numof_peers;

static esp_now_peer_info_t peer_info;

//-----------------------------------------------------------------------------
void espnet_init(){
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
void espnet_task(void * pvParameters ){
  static uint32_t last_search = millis();
  static int32_t search_start = -1;
  while(1){
    // Searching for host
    if(espnet_config.mode == MODE_SEARCHING){
      if(millis() - last_search > ESPNET_SEARCH_INTERVAL){
        if(search_start == -1){
          search_start = millis();
        }
        if((millis() - search_start > ESPNET_TIMEOUT_SEARCH)){
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

    // Check for lost connections
    if(espnet_config.mode == MODE_HOST){
      for(uint8_t i = 0; i < numof_peers; i++){
        // Remove peers that haven't responded for a while
        if(millis() - peer_list[i].last_response > ESPNET_TIMEOUT_CONLOST){
          esp_now_del_peer(peer_list[i].mac);
          for(uint8_t j = i; j < numof_peers - 1; j++){
            peer_list[j] = peer_list[j + 1];
          }
          numof_peers--;
        }
        // Send ping to peers that haven't responded for a while
        else if(millis() - peer_list[i].last_response > ESPNET_TIMEOUT_PING){
          uint8_t packet[] = {PACKET_REQ_PING};
          esp_now_send(peer_list[i].mac, packet, sizeof(packet));
        }
      }
    }

    // Check for lost connection to host
    if(espnet_config.mode == MODE_CLIENT){
      if(millis() - espnet_config.last_response > ESPNET_TIMEOUT_CONLOST){
        espnet_config.mode = MODE_SEARCHING;
        search_start = -1;
        esp_now_del_peer(espnet_config.host_mac);
      }
    }

    vTaskDelay(1);
  }
}


//-----------------------------------------------------------------------------
// Send data to 
void espnet_send(uint8_t id, uint8_t *data, uint32_t len){
  for(uint8_t i = 0; i < numof_peers; i++){
    if(peer_list[i].id == id){
      esp_err_t res = esp_now_send(peer_list[i].mac, data, len);
      //...
      if(res != ESP_OK){
        serial_send_slip(CMD_RSP_ESPNET_ERROR);
        serial_send_slip(res);
        serial_end_slip();
      } 
      break;
    }
  }
}

//-----------------------------------------------------------------------------
// Check if the id is already in the peer list
uint8_t espnet_check_id(uint8_t id){
  for(uint8_t i = 0; i < numof_peers; i++){
    if(peer_list[i].id == id){
      return 1;
    }
  }
  return 0;
}

//-----------------------------------------------------------------------------
void espnow_recv_cb(const uint8_t *addr, const uint8_t *data, int len){
  ESPNET_PACKETS tag = (ESPNET_PACKETS)data[0];
  data ++;
  len --;
  // Update last response times
  if(espnet_config.mode == MODE_HOST){
    for(uint8_t i = 0; i < numof_peers; i++){
      if(memcmp(peer_list[i].mac, addr, 6) == 0){
        peer_list[i].last_response = millis();
        break;
      }
    }
  }
  if(espnet_config.mode == MODE_CLIENT){
    espnet_config.last_response = millis();
  }
  //...
  switch (tag){
    //-------------------------------------------------------------------------
    case PACKET_REQ_PING:
    {
      uint8_t packet[] = {PACKET_RSP_PONG, espnet_config.id};
      esp_now_send(addr, packet, sizeof(packet));
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
          memcpy(peer_info.peer_addr, addr, 6);
          peer_info.channel = 0;
          peer_info.encrypt = false;
          esp_now_add_peer(&peer_info);
          peer_list[numof_peers].id = numof_peers + 1;
          memcpy(peer_list[numof_peers].mac, addr, 6);
          peer_list[numof_peers].mode = MODE_CLIENT;
          peer_list[numof_peers].last_response = millis();
          uint8_t packet[] = {PACKET_RSP_JOIN, peer_list[numof_peers].id};
          esp_now_send(addr, packet, sizeof(packet));
          numof_peers++;
        }
        else{
          uint8_t packet[] = {PACKET_RSP_JOIN, 0};
          esp_now_send(addr, packet, sizeof(packet));
        }
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_JOIN:
    {
      if(data[0] > 0){
        espnet_config.id = data[0];
        espnet_config.mode = MODE_CLIENT;
        espnet_config.last_response = millis();
        memcpy(espnet_config.host_mac, addr, 6);
        // add peer
        memcpy(peer_info.peer_addr, addr, 6);
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
          if(memcmp(peer_list[i].mac, addr, 6) == 0){
            esp_now_del_peer(peer_list[i].mac);
            for(uint8_t j = i; j < numof_peers - 1; j++){
              peer_list[j] = peer_list[j + 1];
            }
            numof_peers--;
            break;
          }
        }
        uint8_t packet[] = {PACKET_RSP_LEAVE};
        esp_now_send(addr, packet, sizeof(packet));
      }
      if(espnet_config.mode == MODE_CLIENT){
        espnet_config.mode = MODE_NONE;
        memset(espnet_config.host_mac, 0, 6);
        uint8_t packet[] = {PACKET_RSP_LEAVE};
        esp_now_send(addr, packet, sizeof(packet));
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_POINTS:
    {
      uint8_t *packet = (uint8_t*)malloc(sizeof(point_rect_t)*tracker_points_len+2);
      packet[0] = PACKET_RSP_POINTS;
      packet[1] = espnet_config.id;
      memcpy(packet+2, tracker_points_rect, sizeof(point_rect_t)*tracker_points_len);
      esp_now_send(addr, packet, sizeof(point_rect_t)*tracker_points_len+2);
      free(packet);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_POINTS:
    {
      uint8_t *packet = (uint8_t*)malloc(2+len-1);
      packet[0] = CMD_RSP_POINTS;
      packet[1] = data[0];
      memcpy(packet+2, data+1, len-1);
      serial_send_slip(packet, 2+len-1);
      serial_end_slip();
      free(packet);
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_FCOUNT:
    {
      if(espnet_config.mode == MODE_CLIENT){
        uint8_t *packet = (uint8_t*)malloc(10);
        packet[0] = PACKET_RSP_FCOUNT;
        packet[1] = espnet_config.id;
        memcpy(packet+2, &tracker_frame_count, 8);
        esp_now_send(addr, packet, 10);
        free(packet);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_RSP_FCOUNT:
    {
      uint8_t rq_from = data[0];
      if(espnet_config.mode == MODE_HOST){
        uint8_t *packet = (uint8_t*)malloc(10);
        packet[0] = CMD_RSP_FCOUNT;
        packet[1] = rq_from;
        memcpy(packet+2, &data[1], 8);
        serial_send_slip(packet, 10);
        serial_end_slip();
        free(packet);
      }
    }break;
    //-------------------------------------------------------------------------
    case PACKET_REQ_ESPNET_CONFIG:
    {
      if(espnet_config.mode == MODE_CLIENT){
        uint8_t *packet = (uint8_t*)malloc(sizeof(espnet_config)+1);
        packet[0] = PACKET_RSP_ESPNET_CONFIG;
        memcpy(packet+1, &espnet_config, sizeof(espnet_config));
        esp_now_send(addr, packet, sizeof(espnet_config)+1);
        free(packet);
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
        uint8_t *packet = (uint8_t*)malloc(2+len+4);
        uint32_t value = CONFIGS.getInt(req_key);
        packet[0] = PACKET_RSP_CONFIG;
        packet[1] = espnet_config.id;
        memcpy(packet+2, req_key, len);
        memcpy(packet+2+len, &value, 4);
        esp_now_send(addr, packet, 2+len+4);
        free(packet);
      }else{
        uint8_t *packet = (uint8_t*)malloc(2+len);
        packet[0] = PACKET_RSP_ERROR;
        packet[1] = espnet_config.id;
        memcpy(packet+2, req_key, len);
        esp_now_send(addr, packet, 2+len);
        free(packet);
      }
    }
    //-------------------------------------------------------------------------
    case PACKET_RSP_CONFIG:
    {
      if(espnet_config.mode == MODE_HOST){
        serial_send_slip((uint8_t)CMD_RSP_CONFIG);
        serial_send_slip((uint8_t *)data, len);
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
      }else{
        uint8_t *packet = (uint8_t*)malloc(2+len-4);
        packet[0] = PACKET_RSP_ERROR;
        packet[1] = espnet_config.id;
        memcpy(packet+2, req_key, len-4);
        esp_now_send(addr, packet, 2+len-4);
        free(packet);
      }
    }break;
    //-------------------------------------------------------------------------
    default:
    {

    }break;
  }
}
