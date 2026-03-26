#pragma once
#include <ESP32Servo.h>

class ServoController {
public:
  static const uint8_t COUNT = 4;

  ServoController(const int pins[COUNT], int minUs, int maxUs)
    : _minUs(minUs), _maxUs(maxUs) {
    for (int i = 0; i < COUNT; i++) _pins[i] = pins[i];
  }

  void begin() {
    for (int i = 0; i < COUNT; i++) {
      _servos[i].setPeriodHertz(50);
      _servos[i].attach(_pins[i], _minUs, _maxUs);
      move(i, 90);
    }
  }

  void move(uint8_t index, int angle) {
    if (index >= COUNT) return;
    angle = constrain(angle, 0, 180);
    _angles[index] = angle;
    _servos[index].write(angle);
    Serial.printf("Servo %d -> %d deg\n", index, angle);
  }

  int getAngle(uint8_t index) const {
    if (index >= COUNT) return 0;
    return _angles[index];
  }

private:
  Servo   _servos[COUNT];
  int     _pins[COUNT];
  int     _angles[COUNT] = {90, 90, 90, 90};
  int     _minUs, _maxUs;
};
