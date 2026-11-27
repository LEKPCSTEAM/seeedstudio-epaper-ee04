#include "hal/Battery/BatteryLevel.h"

#include <cstddef>

namespace {

struct OcvPoint {
  uint16_t mv;
  uint8_t percent;
};

// Typical LiPo (LiCoO2/graphite) open-circuit voltage at rest, 25 C.
constexpr OcvPoint kOcvCurve[] = {
    {3270, 0},  {3610, 5},  {3690, 10}, {3710, 15}, {3730, 20},
    {3750, 25}, {3770, 30}, {3790, 35}, {3800, 40}, {3820, 45},
    {3840, 50}, {3850, 55}, {3870, 60}, {3910, 65}, {3950, 70},
    {3980, 75}, {4020, 80}, {4080, 85}, {4110, 90}, {4150, 95},
    {4200, 100},
};
constexpr size_t kOcvPoints = sizeof(kOcvCurve) / sizeof(kOcvCurve[0]);

// One sample per minute: about a 4 minute time constant.
constexpr float kAlpha = 0.25f;

} // namespace

void BatteryLevel::add(uint32_t millivolts) {
  const float sample = static_cast<float>(millivolts);
  if (!valid_) {
    filteredMv_ = sample;
    valid_ = true;
    return;
  }
  filteredMv_ += kAlpha * (sample - filteredMv_);
}

uint32_t BatteryLevel::millivolts() const {
  return static_cast<uint32_t>(filteredMv_ + 0.5f);
}

uint8_t BatteryLevel::percent() const {
  return percentFromRestingMv(millivolts());
}

uint8_t BatteryLevel::percentFromRestingMv(uint32_t millivolts) {
  if (millivolts <= kOcvCurve[0].mv) {
    return 0;
  }
  for (size_t i = 1; i < kOcvPoints; ++i) {
    const OcvPoint &hi = kOcvCurve[i];
    if (millivolts > hi.mv) {
      continue;
    }
    const OcvPoint &lo = kOcvCurve[i - 1];
    const float t = static_cast<float>(millivolts - lo.mv) / (hi.mv - lo.mv);
    return static_cast<uint8_t>(lo.percent + t * (hi.percent - lo.percent) + 0.5f);
  }
  return 100;
}
