#pragma once

#include <Seeed_GFX.h>

namespace WeatherIcon {

enum class Kind : uint8_t {
  kClear,
  kPartlyCloudy,
  kOvercast,
  kFog,
  kDrizzle,
  kRain,
  kSnow,
  kThunder,
};

Kind kindFromCode(int16_t wmoCode);
const char *describe(int16_t wmoCode);

// Draws inside a size x size box centred on (cx, cy).
void draw(Seeed_GFX &gfx, int16_t cx, int16_t cy, int16_t size,
          int16_t wmoCode, bool isDay);

} // namespace WeatherIcon
