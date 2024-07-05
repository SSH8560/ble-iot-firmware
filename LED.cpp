#include "LED.h"
#include <Arduino.h>

LED::LED(int pin) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
}

void LED::on() {
  digitalWrite(pin, HIGH);
}

void LED::off() {
  digitalWrite(pin, LOW);
}

void LED::toggle() {
  digitalWrite(pin, !digitalRead(pin));
}
