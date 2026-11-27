#pragma once

#include <Seeed_GFX.h>

namespace DotClock {

constexpr int16_t kColumns = 27;
constexpr int16_t kRows = 7;

// Draws HH:MM from the top-left corner (x, y).
void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t pitch, int16_t radius,
          int hour, int minute);

int16_t width(int16_t pitch, int16_t radius);
int16_t height(int16_t pitch, int16_t radius);

} // namespace DotClock
