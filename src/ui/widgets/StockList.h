#pragma once

#include <Seeed_GFX.h>

#include "app/model/DashboardData.h"

namespace StockList {

constexpr int16_t kTitleH = 28;

// Header shows "No WiFi" instead of the update time when !online.
void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, int16_t h,
          const StocksData &stocks, bool online);

} // namespace StockList
