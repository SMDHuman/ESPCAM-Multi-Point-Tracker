#include <Arduino.h>
#include "serial_com.h"
#include "espnet_handler.h"
#include "camera.h"
#include "tracker.h"

void setup() {
  serial_init();
  espnet_init();
  camera_init();
  tracker_init();
}

void loop() {
  espnet_task();
  camera_task();
  serial_task();
  tracker_task();
  delay(1);
}