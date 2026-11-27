#pragma once

#include <cstdint>

class BatteryLevel {
public:
  void add(uint32_t millivolts);
  bool valid() const { return valid_; }
  uint32_t millivolts() const;
  uint8_t percent() const;

  // Single-cell LiPo state of charge from resting voltage, 0..100.
  static uint8_t percentFromRestingMv(uint32_t millivolts);

private:
  float filteredMv_ = 0;
  bool valid_ = false;
};
