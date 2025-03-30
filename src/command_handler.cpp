//-----------------------------------------------------------------------------
// File: command_handler.cpp
// Last modified: 29/03/2025
//-----------------------------------------------------------------------------
#include "command_handler.h"
#include <Arduino.h>
#include "serial_com.h"
#include "tracker.h"
#include "espnet_handler.h"

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
    case CMD_PING:
    {
      serial_send_slip(RSP_PONG);
      serial_end_slip();
    }break;
    case CMD_RQ_FCOUNT:
    {
      uint8_t rq_from = data[0];
      if(rq_from == 0){
        uint8_t *packet = (uint8_t*)malloc(10);
        packet[0] = RSP_FCOUNT;
        packet[1] = rq_from;
        memcpy(packet+2, &tracker_frame_count, 8);
        serial_send_slip(packet, 10);
        serial_end_slip();
        free(packet);
      }else{
        if(espnet_check_id(rq_from)){
          uint8_t pck[1] = {PACKET_REQ_FCOUNT};
          espnet_send(rq_from, pck, 1);
        }
      }
    }break;
    case CMD_RQ_PEERCOUNT:
    {
      uint8_t *packet = (uint8_t*)malloc(2);
      packet[0] = RSP_PEERCOUNT;
      packet[1] = numof_peers;
      serial_send_slip(packet, 2);
      serial_end_slip();
      free(packet);
    }break;
    case CMD_RQ_PEERLIST:
    {
      uint8_t *packet = (uint8_t*)malloc(numof_peers*7+1);
      packet[0] = RSP_PEERLIST;
      for(uint8_t i = 0; i < numof_peers; i++){
        packet[1+i*7] = peer_list[i].id;
        memcpy(packet+1+i*7+1, peer_list[i].mac, 6);
      }
      serial_send_slip(packet, numof_peers*7+1);
      serial_end_slip();
      free(packet);
    }break;
    case CMD_RQ_POINTS:
    {
      uint8_t rq_from = data[0];
      if(rq_from == 0){
        uint8_t *packet = (uint8_t*)malloc(2+tracker_points_len*sizeof(point_rect_t));
        packet[0] = RSP_POINTS;
        packet[1] = 0;
        for(uint8_t i = 0; i < tracker_points_len; i++){
          memcpy(packet+2+i*sizeof(point_rect_t), &tracker_points_rect[i], sizeof(point_rect_t));
        }
        serial_send_slip(packet, 2+tracker_points_len*sizeof(point_rect_t));
        serial_end_slip();
        free(packet);
      }else{
        if(espnet_check_id(rq_from)){
          uint8_t pck[1] = {PACKET_REQ_POINTS};
          espnet_send(rq_from, pck, 1);
        }
      }
    }break;
  }
}
