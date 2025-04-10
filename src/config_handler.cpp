//-----------------------------------------------------------------------------
// config_handler.cpp
//-----------------------------------------------------------------------------
#include <Arduino.h>
#include "config_handler.h"
#include <Preferences.h>
#include "espnet.h"
#include "serial_com.h"
#include "tracker.h"
#include "camera_handler.h"

Preferences CONFIGS;

//-----------------------------------------------------------------------------
void config_init(){
  CONFIGS.begin(CONFIGS_NAME, false);

  // Set default values if the config_restore_default flag is true
  if(CONFIGS.getInt("cfg_restore", true)){
    CONFIGS.putInt("cfg_restore", false);
    CONFIGS.putInt("cam_brightness", 0);
    CONFIGS.putInt("cam_contrast", 0);
    CONFIGS.putInt("cam_saturation", 0);
    CONFIGS.putInt("cam_spec_effect", 2);
    CONFIGS.putInt("cam_whitebal", 1);
    CONFIGS.putInt("cam_awb_gain", 1);
    CONFIGS.putInt("cam_wb_mode", 0);
    CONFIGS.putInt("cam_expo_ctrl", 1);
    CONFIGS.putInt("cam_aec2", 0);
    CONFIGS.putInt("cam_ae_level", 0);
    CONFIGS.putInt("trk_filter_min", 230);
    CONFIGS.putInt("trk_erode", 1);
    CONFIGS.putInt("trk_erode_mul", 4);
    CONFIGS.putInt("trk_erode_div", 4);
    CONFIGS.putInt("trk_dilate", 6);
    CONFIGS.putInt("trk_flip_x", 0);
    CONFIGS.putInt("trk_flip_y", 0);
    CONFIGS.putInt("serial_baudrate", 115200);
    CONFIGS.putInt("espnet_mode", MODE_NONE);
    // Blink led 5 times to indicate default values have been set
    for(uint8_t i = 0; i < 5; i++){
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
  }
}
//-----------------------------------------------------------------------------
void config_task(){
}

//-----------------------------------------------------------------------------
void config_reload(){
  tracker_load_configs();
  camera_load_configs();
  espnet_load_configs();
  serial_load_configs();
}