//-----------------------------------------------------------------------------
// File: command_handler.cpp
// Last modified: 29/03/2025
//-----------------------------------------------------------------------------
#include "command_handler.h"
#include <Arduino.h>
#include "serial_com.h"
#include "tracker.h"
#include "espnet.h"
#include "config_handler.h"

//-----------------------------------------------------------------------------
void command_init(){
}

//-----------------------------------------------------------------------------
void command_task(){
}

//-----------------------------------------------------------------------------
void command_parse(uint8_t *msg_data, uint32_t len){
  CMD_PACKET_TYPE_E cmd = (CMD_PACKET_TYPE_E)msg_data[0];
  uint8_t *data = msg_data + 1; 
  len -= 1;
  esp_err_t res;
  switch(cmd){
    case CMD_REQ_PING:
    {
      serial_send_slip(CMD_RSP_PONG);
      serial_end_slip();
    }break;
    case CMD_REQ_FCOUNT:
    {
      uint8_t rq_from = data[0];
      if(rq_from == 0){
        uint8_t *packet = (uint8_t*)malloc(10);
        packet[0] = CMD_RSP_FCOUNT;
        packet[1] = rq_from;
        memcpy(packet+2, &tracker_frame_count, 8);
        serial_send_slip(packet, 10);
        serial_end_slip();
        free(packet);
      }else{
        espnet_send(PACKET_REQ_FCOUNT, rq_from, {}, 0);
      }
    }break;
    case CMD_REQ_PEERCOUNT:
    {
      uint8_t *packet = (uint8_t*)malloc(2);
      packet[0] = CMD_RSP_PEERCOUNT;
      packet[1] = peer_list->length;
      serial_send_slip(packet, 2);
      serial_end_slip();
      free(packet);
    }break;
    case CMD_REQ_PEERLIST:
    {
      uint8_t *packet = (uint8_t*)malloc(peer_list->length*7+1);
      packet[0] = CMD_RSP_PEERLIST;
      for(uint8_t i = 0; i < peer_list->length; i++){
        espnet_config_t peer_config;
        array_get(peer_list, i, &peer_config);
        packet[1+i*7] = peer_config.id;
        memcpy(packet+1+i*7+1, peer_config.mac, 6);
      }
      serial_send_slip(packet, peer_list->length*7+1);
      serial_end_slip();
      free(packet);
    }break;
    case CMD_REQ_POINTS:
    {
      uint8_t rq_from = data[0];
      if(rq_from == 0){
        uint8_t *packet = (uint8_t*)malloc(2+tracker_points_len*sizeof(point_rect_t));
        packet[0] = CMD_RSP_POINTS;
        packet[1] = 0;
        for(uint8_t i = 0; i < tracker_points_len; i++){
          memcpy(packet+2+i*sizeof(point_rect_t), &tracker_points_rect[i], sizeof(point_rect_t));
        }
        serial_send_slip(packet, 2+tracker_points_len*sizeof(point_rect_t));
        serial_end_slip();
        free(packet);
      }else{
        espnet_send(PACKET_REQ_POINTS, rq_from, {}, 0);
      }
    }break;
    case CMD_REBOOT:
    {
      esp_restart();
    }break;
    case CMD_REQ_CONFIG:
    {
      uint8_t rq_from = data[0];
      data++;
      len--;
      if(rq_from == 0){
        char req_key[16] = {0};
        memcpy(req_key, data, len);
        if(CONFIGS.isKey(req_key)){
          uint8_t *packet = (uint8_t*)malloc(2+len+4);
          uint32_t value = CONFIGS.getInt(req_key);
          packet[0] = CMD_RSP_CONFIG;
          packet[1] = 0;
          memcpy(packet+2, req_key, len);
          memcpy(packet+2+len, &value, 4);
          serial_send_slip(packet, 2+len+4);
          serial_end_slip();
          free(packet);
        }else{
          uint8_t *packet = (uint8_t*)malloc(2+len);
          packet[0] = CMD_RSP_ERROR;
          packet[1] = 0;
          memcpy(packet+2, req_key, len);
          serial_send_slip(packet, 2+len);
          serial_end_slip();
          free(packet);
        }
      }else{
        espnet_send(PACKET_REQ_CONFIG, rq_from, data, len);
      }
    }break;
    case CMD_SET_CONFIG:
    {
      uint8_t rq_from = data[0];
      data++;
      len--;
      if(rq_from == 0){
        char req_key[16] = {0};
        memcpy(req_key, data, len-4);
        uint32_t value = *(uint32_t*)(data+len-4);
        if(CONFIGS.isKey(req_key)){
          CONFIGS.putInt(req_key, value);
          serial_send_slip(CMD_RSP_CONFIG);
          serial_send_slip(0);
          serial_end_slip();
        }else{
          uint8_t *packet = (uint8_t*)malloc(2+len-4);
          packet[0] = CMD_RSP_ERROR;
          packet[1] = 0;
          memcpy(packet+2, req_key, len-4);
          serial_send_slip(packet, 2+len-4);
          serial_end_slip();
          free(packet);
        }
      }else{
        espnet_send(PACKET_SET_CONFIG, rq_from, data, len);
      }
    }break;
    case CMD_REQ_RELOAD_CONFIG:
    {
      uint8_t rq_from = data[0];
      if(rq_from == 0){
        config_reload();
        serial_send_slip((uint8_t)CMD_RSP_RELOAD_CONFIG);
        serial_send_slip((uint8_t)0);
        serial_end_slip();
      }else{
        espnet_send(PACKET_REQ_RELOAD_CONFIG, rq_from);
      }
    }
  } 
}
