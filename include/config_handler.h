//-----------------------------------------------------------------------------
// config_handler.h
//-----------------------------------------------------------------------------
#ifndef CONFIG_HANDLER_H
#define CONFIG_HANDLER_H
#include <Arduino.h>
#include <Preferences.h>

extern Preferences CONFIGS;
#define CONFIGS_NAME "espcam-tracker"

const String CONFIGS_KEYS[] = {
    "config_restore_default",
    "camera_brightness",
    "camera_contrast",
    "camera_saturation",
    "camera_special_effect",
    "camera_whitebal",
    "camera_awb_gain",
    "camera_wb_mode",
    "camera_exposure_ctrl",
    "camera_aec2",
    "camera_ae_level",
    "tracker_filter_min",
    "tracker_erode",
    "tracker_erode_mul",
    "tracker_erode_div",
    "tracker_dilate",
    "serial_baudrate"
};

void config_init();
void config_task();
void config_reload();

#endif