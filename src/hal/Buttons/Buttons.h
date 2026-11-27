#pragma once

#include <Arduino.h>

class Buttons {
public:
  static constexpr uint8_t kCount = 3;
  static constexpr uint8_t kKey0 = 1u << 0;
  static constexpr uint8_t kKey1 = 1u << 1;
  static constexpr uint8_t kKey2 = 1u << 2;

  void begin();
  // Bitmask of kKeyN pressed since the previous call, debounced.
  uint8_t pollPressed();

private:
  bool stable_[kCount] = {};
  bool lastRaw_[kCount] = {};
  uint32_t changedAtMs_[kCount] = {};
};
