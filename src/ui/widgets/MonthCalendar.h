#pragma once

#include <Seeed_GFX.h>
#include <ctime>

namespace MonthCalendar {

constexpr int16_t kHeaderH = 32;
constexpr int16_t kWeekdayRowH = 24;
constexpr int16_t kCellH = 31;

// Draws header bar, weekday row and day grid; returns the bottom y.
int16_t draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, const tm &now);

} // namespace MonthCalendar
