#include <Arduino.h>
#include "serial_com.h"
#include "espnow_handler.h"
#include "camera.h"
#include "tracker.h"

void setup() {
  serial_init();
  espnow_init();
  camera_init();
  tracker_init();
}

void loop() {
  espnow_task();
  camera_task();
  serial_task();
  tracker_task();
}