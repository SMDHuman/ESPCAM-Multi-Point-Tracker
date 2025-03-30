#include <Arduino.h>
#include "serial_com.h"
#include "espnet_handler.h"
#include "camera.h"
#include <freertos/task.h>

void setup() {
  serial_init();
  espnet_init();
  camera_init();
  //tracker_init();

  xTaskCreatePinnedToCore(espnet_task, "ESP-NET", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(serial_task, "Serial", 8192, NULL, 1, NULL, 0);
  //xTaskCreatePinnedToCore(tracker_task, "Tracker", 8192, NULL, 1, NULL, 1);
}

void loop() {
  //espnet_task();
  camera_task(0);
  //serial_task();
  //tracker_task();
  //delay(1);
}