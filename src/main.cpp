#include <Arduino.h>
#include "serial_com.h"

void setup() {
  serial_init();
}

void loop() {
  serial_task();
}