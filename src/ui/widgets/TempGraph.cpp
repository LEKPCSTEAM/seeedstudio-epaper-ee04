#include "ui/widgets/TempGraph.h"

#include <cmath>
#include <cstdio>

#include "ui/i18n/Strings.h"
#include "ui/widgets/Fonts.h"
#include "ui/widgets/TextUtil.h"

namespace TempGraph {

namespace {

constexpr int16_t kAxisW = 34;
constexpr int16_t kRightPad = 34;
constexpr int16_t kTopLabelH = 27;
constexpr int16_t kBottomLabelH = 22;
constexpr int16_t kPointPad = 6;
constexpr int16_t kDotRadius = 4;
constexpr float kLineWidth = 3.0f;
constexpr int16_t kLabelGap = 6;

void drawTempLabel(Seeed_GFX &gfx, float tempC, int16_t x, int16_t y,
                   TextUtil::Align align, uint8_t datum) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%.0f", tempC);
  const int16_t textW = TextUtil::textWidth(gfx, Fonts::sans9(), buf);
  gfx.setFreeFont(&Fonts::sans9());
  const int16_t r = std::max<int16_t>(3, gfx.fontHeight() / 8);
  const int16_t total = textW + 2 + 2 * r;
  const int16_t sx = (align == TextUtil::kCenter) ? x - total / 2
                     : (align == TextUtil::kRight) ? x - total
                                                   : x;
  TextUtil::drawText(gfx, Fonts::sans9(), buf, sx, y, datum);
  const int16_t cy = (datum == BL_DATUM) ? y - gfx.fontHeight() * 3 / 4
                                         : y - gfx.fontHeight() / 4;
  gfx.drawCircle(sx + textW + 2 + r, cy, r, TFT_BLACK);
  gfx.drawCircle(sx + textW + 2 + r, cy, r - 1, TFT_BLACK);
}

} // namespace

void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, int16_t h,
          const HourlyTemp *points, uint8_t count) {
  if (count < 2) {
    return;
  }
  const int16_t plotL = x + kAxisW;
  const int16_t plotR = x + w - kRightPad;
  const int16_t plotTop = y + kTopLabelH;
  const int16_t baseY = y + h - kBottomLabelH;
  const int16_t pointTop = plotTop + kDotRadius;
  const int16_t pointBottom = baseY - kPointPad - kDotRadius;

  float lo = points[0].tempC;
  float hi = points[0].tempC;
  for (uint8_t i = 1; i < count; ++i) {
    lo = std::min(lo, points[i].tempC);
    hi = std::max(hi, points[i].tempC);
  }
  const float span = hi - lo;

  int16_t px[kHourlyPoints];
  int16_t py[kHourlyPoints];
  for (uint8_t i = 0; i < count && i < kHourlyPoints; ++i) {
    px[i] = plotL + (plotR - plotL) * i / (count - 1);
    py[i] = (span < 0.01f)
                ? (pointTop + pointBottom) / 2
                : pointBottom - static_cast<int16_t>((points[i].tempC - lo) /
                                                     span *
                                                     (pointBottom - pointTop));
  }

  gfx.drawFastHLine(plotL - 8, baseY, plotR - plotL + 16, TFT_BLACK);
  gfx.drawFastHLine(plotL - 8, baseY + 1, plotR - plotL + 16, TFT_BLACK);

  for (uint8_t i = 1; i < count; ++i) {
    TextUtil::thickLine(gfx, px[i - 1], py[i - 1], px[i], py[i], kLineWidth);
  }

  // Thai labels are 3 bytes per character in UTF-8.
  char buf[32];
  for (uint8_t i = 0; i < count; ++i) {
    gfx.fillCircle(px[i], py[i], kDotRadius, TFT_BLACK);
    drawTempLabel(gfx, points[i].tempC, px[i], py[i] - kDotRadius - kLabelGap,
                  TextUtil::kCenter, BL_DATUM);

    if (i == 0) {
      snprintf(buf, sizeof(buf), "%s", Strings::now());
    } else {
      tm t;
      const time_t at = points[i].time;
      if (!localtime_r(&at, &t)) {
        continue;
      }
      snprintf(buf, sizeof(buf), "%02d:00", t.tm_hour);
    }
    TextUtil::drawText(gfx, Fonts::sans9(), buf, px[i], baseY + 3, TC_DATUM);
  }
}

} // namespace TempGraph
