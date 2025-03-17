#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H
#include <Arduino.h>
#include "esp_now.h"
#include "WiFi.h"

//-----------------------------------------------------------------------------
enum ESPNOW_MODES{
    MODE_NONE = 0,
    MODE_HOST,
    MODE_CLIENT
};

//-----------------------------------------------------------------------------
void espnow_init();
void espnow_task();


#endif