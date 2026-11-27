#include "hal/Battery/Battery.h"

#include <algorithm>
#include <iterator>

namespace {

// Schematic v1.2: R28/R29 are 10K 1%, so the pin sees VBAT / 2.
constexpr float kDividerRatio = 2.0f;
// TPS22916 turn-on plus divider settle; Seeed wiki suggests 10 ms.
constexpr uint32_t kSettleMs = 10;
constexpr size_t kSamples = 32;
// Average only the middle half to reject ADC spikes.
constexpr size_t kTrim = kSamples / 4;

} // namespace

void Battery::begin(float calScale) {
  calScale_ = calScale;
  pinMode(BAT_ADC_EN, OUTPUT);
  digitalWrite(BAT_ADC_EN, LOW);
  analogReadResolution(12);
  // 4.2 V / 2 = 2.1 V at the pin needs the 12 dB range.
  analogSetAttenuation(ADC_11db);
}

uint32_t Battery::readMillivolts() {
  digitalWrite(BAT_ADC_EN, HIGH);
  delay(kSettleMs);
  uint32_t samples[kSamples];
  for (uint32_t &s : samples) {
    // eFuse-calibrated, unlike the raw count the Seeed 7.16 formula used.
    s = analogReadMilliVolts(BAT_ADC_PIN);
  }
  digitalWrite(BAT_ADC_EN, LOW);

  std::sort(std::begin(samples), std::end(samples));
  uint32_t sum = 0;
  for (size_t i = kTrim; i < kSamples - kTrim; ++i) {
    sum += samples[i];
  }
  const float pinMv = static_cast<float>(sum) / (kSamples - 2 * kTrim);
  return static_cast<uint32_t>(pinMv * kDividerRatio * calScale_ + 0.5f);
}
