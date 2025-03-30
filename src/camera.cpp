//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "camera.h"
#include "serial_com.h"
#include "tracker.h"

//-----------------------------------------------------------------------------
void camera_init(){
  esp_err_t err = esp_camera_init(&camera_config);
  if(err == -1){
    Serial.println("Failed to initialize camera!!");
    return;
  }
  sensor_t *s = esp_camera_sensor_get();
  // 0 to 6 (0: No Effect, 1: Negative, 2: Grayscale, ...)
  s->set_special_effect(s, 2); 
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

    tracker_push_camera_buffer(fb);
    tracker_process();

    esp_camera_fb_return(fb);
    vTaskDelay(1);
  }
}
