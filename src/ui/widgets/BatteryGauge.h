#pragma once

#include <Seeed_GFX.h>

namespace BatteryGauge {

// Icon plus percent text, right edge at xRight; percent < 0 draws "--%".
// Returns the left edge used.
int16_t draw(Seeed_GFX &gfx, int16_t xRight, int16_t yMid, int percent,
             uint16_t color);

} // namespace BatteryGauge
