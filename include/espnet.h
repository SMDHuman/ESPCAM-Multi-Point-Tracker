#ifndef ESPNET_HANDLER_H
#define ESPNET_HANDLER_H

#include <Arduino.h>
#include "p_array.h"

#define LED_BUILTIN 33

#define MAX_PEERS 5
#define ESPNET_SEARCH_INTERVAL 200
#define ESPNET_TIMEOUT_SEARCH 3000
#define ESPNET_TIMEOUT_PING 1000
#define ESPNET_TIMEOUT_CONLOST 4000

//-----------------------------------------------------------------------------
enum ESPNET_PACKETS: uint8_t{
    PACKET_REQ_PING = 0,
    PACKET_RSP_PONG,
    PACKET_REQ_LEDTOGGLE,
    PACKET_REQ_POINTS,
    PACKET_RSP_POINTS,
    PACKET_REQ_FCOUNT,
    PACKET_RSP_FCOUNT,
    PACKET_REQ_ESPNET_CONFIG,
    PACKET_RSP_ESPNET_CONFIG,
    PACKET_SET_ESPNET_CONFIG,
    PACKET_RSP_ERROR,
    PACKET_REQ_JOIN,
    PACKET_RSP_JOIN_ACCEPT,
    PACKET_RSP_JOIN_DENY,
    PACKET_REQ_LEAVE,
    PACKET_RSP_LEAVE,
    PACKET_REQ_CONFIG,
    PACKET_RSP_CONFIG,
    PACKET_SET_CONFIG,
    PACKET_REQ_RELOAD_CONFIG,
    PACKET_RSP_RELOAD_CONFIG,
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
    uint32_t last_response;
};

extern espnet_config_t espnet_config;
extern array *peer_list;

//-----------------------------------------------------------------------------
void espnet_init();
void espnet_task(void * pvParameters);
void espnet_load_configs();
void espnet_send(ESPNET_PACKETS tag, uint8_t id, uint8_t *data, uint32_t len);
void espnet_send(ESPNET_PACKETS tag, uint8_t id);
uint8_t espnet_check_id(uint8_t id);
    

#endif