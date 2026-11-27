#pragma once

#include <Seeed_GFX.h>

namespace TextUtil {

enum Align : uint8_t { kLeft, kCenter, kRight };

void drawText(Seeed_GFX &gfx, const GFXfont &font, const char *text, int16_t x,
              int16_t y, uint8_t datum, uint16_t color = TFT_BLACK);

// Handles UTF-8 with Thai runs; measures exactly what drawText draws.
int16_t textWidth(Seeed_GFX &gfx, const GFXfont &font, const char *text);

// Text, a drawn degree ring, then an optional suffix; y is the vertical middle.
int16_t drawDegreeText(Seeed_GFX &gfx, const GFXfont &font, const char *text,
                       const char *suffix, int16_t x, int16_t y, Align align,
                       uint16_t color = TFT_BLACK);

void thickLine(Seeed_GFX &gfx, float x0, float y0, float x1, float y1,
               float thickness, uint16_t color = TFT_BLACK);

void formatNumber(char *out, size_t size, double value, uint8_t decimals);

} // namespace TextUtil
