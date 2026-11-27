#pragma once

#include <Seeed_GFX.h>

#include "app/model/DashboardData.h"

namespace TempGraph {

void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, int16_t h,
          const HourlyTemp *points, uint8_t count);

} // namespace TempGraph
