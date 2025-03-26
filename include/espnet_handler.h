#ifndef ESPNET_HANDLER_H
#define ESPNET_HANDLER_H

#include "esp_now.h"
#include "WiFi.h"

#define MAX_PEERS 5
#define ESPNET_TIMEOUT 3000

//-----------------------------------------------------------------------------
enum ESPNET_PACKETS: uint8_t{
    PACKET_REQ_PING = 0,
    PACKET_RSP_PONG,
    PACKET_REQ_LEDTOGGLE,
    PACKET_REQ_POINTS,
    PACKET_RSP_POINTS,
    PACKET_REQ_FCOUNT,
    PACKET_RSP_FCOUNT,
    PACKET_REQ_CONFIG,
    PACKET_RSP_CONFIG,
    PACKET_SET_CONFIG,
    PACKET_RSP_ERROR,
    PACKET_REQ_JOIN,
    PACKET_RSP_JOIN,
    PACKET_REQ_LEAVE,
    PACKET_RSP_LEAVE,
};

enum ESPNET_MODES: uint8_t{
    MODE_NONE = 0,
    MODE_HOST,
    MODE_CLIENT,
    MODE_SEARCHING
};

struct espnet_config_t{
    uint8_t id;
    ESPNET_MODES mode;
    uint8_t mac[6]; 
    uint8_t host_mac[6]; 
};

extern espnet_config_t peer_list[MAX_PEERS];
extern uint8_t numof_peers;
extern uint8_t device_id;

//-----------------------------------------------------------------------------
void espnet_init();
void espnet_task();


#endif