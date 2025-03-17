#include <Arduino.h>
#include "serial_com.h"
#include "espnow_handler.h"

void setup() {
  serial_init();
  espnow_init();
}

void loop() {
  serial_task();
  espnow_task();
}