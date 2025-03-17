#ifndef ESPNOW_HANDLER_H
#define ESPNOW_HANDLER_H

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