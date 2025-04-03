#include <Arduino.h>
#include "serial_com.h"
#include "espnet_handler.h"
#include "camera.h"
#include "config_handler.h"
#include "tracker.h"
#include <freertos/task.h>

void setup() {
  //...
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);
  
  //... 
  config_init();
  serial_init();
  espnet_init();
  tracker_init();
  camera_init();
  
  xTaskCreatePinnedToCore(espnet_task, "ESP-NET", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(serial_task, "Serial", 8192, NULL, 1, NULL, 0);
  //xTaskCreatePinnedToCore(tracker_task, "Tracker", 8192, NULL, 1, NULL, 1);
}

void loop() {
  //espnet_task();
  camera_task(0);
}