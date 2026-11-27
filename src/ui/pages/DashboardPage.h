#pragma once

#include <Seeed_GFX.h>
#include <ctime>

#include "app/model/DashboardData.h"

namespace DashboardPage {

struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

void render(Seeed_GFX &gfx, const DashboardData &data, const tm &now);

// Region redrawn each minute by partial refresh; x and w are multiples of 8.
Rect clockRect();
void renderClock(Seeed_GFX &gfx, const tm &now);

} // namespace DashboardPage
