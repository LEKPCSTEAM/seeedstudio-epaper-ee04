#include "ui/widgets/StockList.h"

#include <cmath>
#include <cstdio>

#include "ui/i18n/Strings.h"
#include "ui/widgets/Fonts.h"
#include "ui/widgets/TextUtil.h"

namespace StockList {

namespace {

constexpr int16_t kPadX = 4;
constexpr int16_t kPriceRightFrac = 62;
constexpr int16_t kTriHalf = 6;
constexpr int16_t kTriGap = 6;

void drawRow(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, int16_t h,
             const Quote &q) {
  const int16_t midY = y + h / 2;
  const char *label = q.label[0] ? q.label : q.symbol;
  TextUtil::drawText(gfx, Fonts::bold9(), label, x + kPadX, midY, ML_DATUM);

  if (!q.valid) {
    TextUtil::drawText(gfx, Fonts::sans9(), "--", x + w * kPriceRightFrac / 100,
                       midY, MR_DATUM);
    TextUtil::drawText(gfx, Fonts::sans9(), "--", x + w - kPadX, midY,
                       MR_DATUM);
    return;
  }

  char buf[24];
  TextUtil::formatNumber(buf, sizeof(buf), q.price, 2);
  TextUtil::drawText(gfx, Fonts::sans9(), buf, x + w * kPriceRightFrac / 100,
                     midY, MR_DATUM);

  if (q.previousClose <= 0) {
    TextUtil::drawText(gfx, Fonts::sans9(), "--", x + w - kPadX, midY,
                       MR_DATUM);
    return;
  }
  const float pct = (q.price - q.previousClose) / q.previousClose * 100.0f;
  snprintf(buf, sizeof(buf), "%.2f%%", fabsf(pct));
  const int16_t textW = TextUtil::textWidth(gfx, Fonts::sans9(), buf);
  const int16_t textRight = x + w - kPadX;
  TextUtil::drawText(gfx, Fonts::sans9(), buf, textRight, midY, MR_DATUM);

  const int16_t triCx = textRight - textW - kTriGap - kTriHalf;
  if (pct >= 0) {
    gfx.fillTriangle(triCx - kTriHalf, midY + kTriHalf - 1, triCx + kTriHalf,
                     midY + kTriHalf - 1, triCx, midY - kTriHalf, TFT_BLACK);
  } else {
    gfx.fillTriangle(triCx - kTriHalf, midY - kTriHalf + 1, triCx + kTriHalf,
                     midY - kTriHalf + 1, triCx, midY + kTriHalf, TFT_BLACK);
  }
}

} // namespace

void draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, int16_t h,
          const StocksData &stocks, bool online) {
  const char *markets = Strings::markets();
  TextUtil::drawText(gfx, Fonts::bold12(), markets, x + kPadX, y + kTitleH / 2,
                     ML_DATUM);
  const int16_t marketsW = TextUtil::textWidth(gfx, Fonts::bold12(), markets);

  char clock[8] = "--:--";
  if (stocks.updatedAt > 0) {
    tm t;
    const time_t at = stocks.updatedAt;
    if (localtime_r(&at, &t)) {
      snprintf(clock, sizeof(clock), "%02d:%02d", t.tm_hour, t.tm_min);
    }
  }
  char updated[48];
  snprintf(updated, sizeof(updated), "%s %s", Strings::updated(), clock);
  const char *shown = !online ? Strings::noWifi()
                      : TextUtil::textWidth(gfx, Fonts::sans9(), updated) + marketsW + 3 * kPadX <= w
                          ? updated
                          : clock;
  TextUtil::drawText(gfx, Fonts::sans9(), shown, x + w - kPadX, y + kTitleH / 2,
                     MR_DATUM);
  gfx.drawFastHLine(x, y + kTitleH, w, TFT_BLACK);

  const int16_t rowH = (h - kTitleH) / kQuoteCount;
  for (uint8_t i = 0; i < kQuoteCount; ++i) {
    drawRow(gfx, x, y + kTitleH + 1 + i * rowH, w, rowH, stocks.quotes[i]);
  }
}

} // namespace StockList
