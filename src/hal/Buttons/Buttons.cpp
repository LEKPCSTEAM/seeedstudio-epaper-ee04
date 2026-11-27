#include "hal/Buttons/Buttons.h"

namespace {

constexpr uint8_t kPins[Buttons::kCount] = {KEY0_PIN, KEY1_PIN, KEY2_PIN};
constexpr uint32_t kDebounceMs = 30;

bool isDown(uint8_t pin) { return digitalRead(pin) == LOW; }

} // namespace

void Buttons::begin() {
  for (uint8_t i = 0; i < kCount; ++i) {
    pinMode(kPins[i], INPUT_PULLUP);
    stable_[i] = lastRaw_[i] = isDown(kPins[i]);
  }
}

uint8_t Buttons::pollPressed() {
  const uint32_t now = millis();
  uint8_t pressed = 0;

  for (uint8_t i = 0; i < kCount; ++i) {
    const bool down = isDown(kPins[i]);
    if (down != lastRaw_[i]) {
      lastRaw_[i] = down;
      changedAtMs_[i] = now;
      continue;
    }
    if (down != stable_[i] && now - changedAtMs_[i] >= kDebounceMs) {
      stable_[i] = down;
      if (down) {
        pressed |= 1u << i;
      }
    }
  }
  return pressed;
}
