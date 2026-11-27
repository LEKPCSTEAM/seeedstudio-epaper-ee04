#include "ui/widgets/MonthCalendar.h"

#include <cstdio>

#include "ui/i18n/Strings.h"
#include "ui/widgets/Fonts.h"
#include "ui/widgets/TextUtil.h"

namespace MonthCalendar {

namespace {

constexpr int16_t kTitlePadX = 8;
// Header width left of the battery gauge drawn by DashboardPage.
constexpr int16_t kTitleMaxW = 176;
constexpr int16_t kTodayW = 32;
constexpr int16_t kTodayH = 26;
constexpr int16_t kTodayRadius = 6;

int daysInMonth(int year, int month) {
  static constexpr uint8_t kDays[12] = {31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};
  const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
  return kDays[month] + ((month == 1 && leap) ? 1 : 0);
}

} // namespace

int16_t draw(Seeed_GFX &gfx, int16_t x, int16_t y, int16_t w, const tm &now) {
  // Thai month names are 3 bytes per character in UTF-8.
  char title[64];
  snprintf(title, sizeof(title), "%s %d", Strings::monthLong(now.tm_mon),
           Strings::displayYear(now.tm_year));
  gfx.fillRect(x, y, w, kHeaderH, TFT_BLACK);
  const GFXfont &titleFont =
      TextUtil::textWidth(gfx, Fonts::bold12(), title) <= kTitleMaxW ? Fonts::bold12()
                                                                      : Fonts::bold9();
  TextUtil::drawText(gfx, titleFont, title, x + kTitlePadX,
                     y + kHeaderH / 2, ML_DATUM, TFT_WHITE);

  const int16_t colW = w / 7;
  const int16_t gridX = x + (w - colW * 7) / 2;
  const int16_t weekdayY = y + kHeaderH;
  for (int i = 0; i < 7; ++i) {
    TextUtil::drawText(gfx, Fonts::bold9(), Strings::weekdayMin(i),
                       gridX + i * colW + colW / 2, weekdayY + kWeekdayRowH / 2,
                       MC_DATUM);
  }
  const int16_t ruleY = weekdayY + kWeekdayRowH;
  gfx.drawFastHLine(x, ruleY, w, TFT_BLACK);

  const int16_t gridY = ruleY + 2;
  const int firstWday = ((now.tm_wday - (now.tm_mday - 1)) % 7 + 7) % 7;
  const int days = daysInMonth(now.tm_year + 1900, now.tm_mon);
  char num[4];
  for (int d = 1; d <= days; ++d) {
    const int slot = firstWday + d - 1;
    const int16_t cx = gridX + (slot % 7) * colW + colW / 2;
    const int16_t cy = gridY + (slot / 7) * kCellH + kCellH / 2;
    snprintf(num, sizeof(num), "%d", d);
    if (d == now.tm_mday) {
      gfx.fillRoundRect(cx - kTodayW / 2, cy - kTodayH / 2, kTodayW, kTodayH,
                        kTodayRadius, TFT_BLACK);
      TextUtil::drawText(gfx, Fonts::bold9(), num, cx, cy, MC_DATUM, TFT_WHITE);
    } else {
      TextUtil::drawText(gfx, Fonts::sans9(), num, cx, cy, MC_DATUM);
    }
  }
  const int rows = (firstWday + days + 6) / 7;
  return gridY + rows * kCellH;
}

} // namespace MonthCalendar
