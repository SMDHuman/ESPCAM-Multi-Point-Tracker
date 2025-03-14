#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H
#include <Arduino.h>
#include "esp_now.h"
#include "WiFi.h"

//-----------------------------------------------------------------------------
enum ESPNOW_MODES{
    MODE_NONE = 0,
    MODE_HOST,
    MODE_CLIENT,
    MODE_REJECTED
};

enum ESPNOW_TAGS{
    TAG_NONE = 0,
    TAG_JOIN,
    TAG_ACCEPT,
    TAG_REJECT
};

//-----------------------------------------------------------------------------
void espnow_init();
void espnow_task();
uint8_t espnow_find_host(int32_t timeout = 1000);
uint8_t espnow_init_host();
void espnow_send(uint8_t *peer_addr, ESPNOW_TAGS tag, uint8_t *data, uint8_t len);


#endif