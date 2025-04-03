//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "camera.h"
#include "serial_com.h"
#include "tracker.h"
#include "espnet_handler.h"

//-----------------------------------------------------------------------------
void camera_init(){
  esp_err_t err = esp_camera_init(&camera_config);
  if(err == -1){
    Serial.println("Failed to initialize camera!!");
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  s->set_special_effect(s, 2);
  //s->set_brightness(s, CONFIGS.getInt("cam_brightness")); // -2 to 2
  //s->set_contrast(s, CONFIGS.getInt("cam_contrast")); // -2 to 2
  //s->set_saturation(s, CONFIGS.getInt("cam_saturation")); // -2 to 2
  //s->set_special_effect(s, CONFIGS.getInt("cam_spec_effect")); // 0 to 7
  //s->set_whitebal(s, CONFIGS.getInt("cam_whitebal")); // 0 to 1
  //s->set_awb_gain(s, CONFIGS.getInt("cam_awb_gain")); // 0 to 1
  //s->set_wb_mode(s, CONFIGS.getInt("cam_wb_mode")); // 0 to 5
  //s->set_exposure_ctrl(s, CONFIGS.getInt("cam_expo_ctrl")); // 0 to 1
  //s->set_aec2(s, CONFIGS.getInt("cam_aec2")); // 0 to 1
  //s->set_ae_level(s, CONFIGS.getInt("cam_ae_level")); // -2 to 2
}
//-----------------------------------------------------------------------------
void camera_task(void * pvParameters){
  while(1) {
    uint64_t task_start = millis();
    camera_fb_t *fb = esp_camera_fb_get(); // get fresh image
    
    if(!fb){
      Serial.println("Couldn't get frame buffer!");
      vTaskDelay(100);
      continue;
    }
    if(espnet_config.mode == MODE_CLIENT){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    tracker_push_camera_buffer(fb);
    tracker_process();

    esp_camera_fb_return(fb);
    vTaskDelay(1);
  }
}
