#include "ui/widgets/BatteryGauge.h"

#include <cstdio>

#include "ui/widgets/Fonts.h"
#include "ui/widgets/TextUtil.h"

namespace BatteryGauge {

namespace {

constexpr int16_t kBodyW = 28;
constexpr int16_t kBodyH = 14;
constexpr int16_t kNubW = 3;
constexpr int16_t kNubH = 6;
constexpr int16_t kGap = 6;

} // namespace

int16_t draw(Seeed_GFX &gfx, int16_t xRight, int16_t yMid, int percent,
             uint16_t color) {
  char text[8];
  if (percent < 0) {
    snprintf(text, sizeof(text), "--%%");
  } else {
    snprintf(text, sizeof(text), "%d%%", percent);
  }
  const int16_t textW = TextUtil::textWidth(gfx, Fonts::sans9(), text);
  TextUtil::drawText(gfx, Fonts::sans9(), text, xRight, yMid, MR_DATUM, color);

  const int16_t bodyX = xRight - textW - kGap - kNubW - kBodyW;
  const int16_t bodyY = yMid - kBodyH / 2;
  gfx.drawRect(bodyX, bodyY, kBodyW, kBodyH, color);
  gfx.drawRect(bodyX + 1, bodyY + 1, kBodyW - 2, kBodyH - 2, color);
  gfx.fillRect(bodyX + kBodyW, yMid - kNubH / 2, kNubW, kNubH, color);

  const int16_t innerW = kBodyW - 6;
  const int16_t fillW =
      static_cast<int16_t>(innerW * constrain(percent, 0, 100) / 100.0f + 0.5f);
  gfx.fillRect(bodyX + 3, bodyY + 3, fillW, kBodyH - 6, color);
  return bodyX;
}

} // namespace BatteryGauge
