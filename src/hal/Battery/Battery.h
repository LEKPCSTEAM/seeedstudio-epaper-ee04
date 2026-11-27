#pragma once

#include <Arduino.h>

class Battery {
public:
  // calScale: multimeter VBAT / reported mV, 1.0 when uncalibrated.
  void begin(float calScale);
  // Battery terminal voltage; call with the radio off for a resting value.
  uint32_t readMillivolts();

private:
  float calScale_ = 1.0f;
};
