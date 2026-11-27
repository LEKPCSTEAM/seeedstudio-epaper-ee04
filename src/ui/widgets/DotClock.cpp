#include "ui/widgets/DotClock.h"

namespace DotClock {

namespace {

constexpr uint8_t kGlyphs[10][kRows] = {
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
    {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E},
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
};

constexpr int16_t kDigitCols[4] = {0, 6, 16, 22};
constexpr int16_t kColonCol = 13;

void drawDigit(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t pitch,
               int16_t radius, int digit, int16_t col0) {
  for (int row = 0; row < kRows; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (kGlyphs[digit][row] & (0x10 >> col)) {
        gfx.fillCircle(x + (col0 + col) * pitch + radius,
                       y + row * pitch + radius, radius, TFT_BLACK);
      }
    }
  }
}

} // namespace

int16_t width(int16_t pitch, int16_t radius) {
  return (kColumns - 1) * pitch + 2 * radius;
}

int16_t height(int16_t pitch, int16_t radius) {
  return (kRows - 1) * pitch + 2 * radius;
}

void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t pitch, int16_t radius,
          int hour, int minute) {
  hour = constrain(hour, 0, 23);
  minute = constrain(minute, 0, 59);
  drawDigit(gfx, x, y, pitch, radius, hour / 10, kDigitCols[0]);
  drawDigit(gfx, x, y, pitch, radius, hour % 10, kDigitCols[1]);
  drawDigit(gfx, x, y, pitch, radius, minute / 10, kDigitCols[2]);
  drawDigit(gfx, x, y, pitch, radius, minute % 10, kDigitCols[3]);
  for (int row : {2, 4}) {
    gfx.fillCircle(x + kColonCol * pitch + radius, y + row * pitch + radius,
                   radius, TFT_BLACK);
  }
}

} // namespace DotClock
