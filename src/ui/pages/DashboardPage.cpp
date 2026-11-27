#include "ui/pages/DashboardPage.h"

#include <cstdio>

#include "ui/i18n/Strings.h"
#include "ui/widgets/BatteryGauge.h"
#include "ui/widgets/DotClock.h"
#include "ui/widgets/Fonts.h"
#include "ui/widgets/MonthCalendar.h"
#include "ui/widgets/StockList.h"
#include "ui/widgets/TempGraph.h"
#include "ui/widgets/TextUtil.h"
#include "ui/widgets/WeatherIcon.h"

namespace {

using TextUtil::drawDegreeText;
using TextUtil::drawText;

// Clock zone
constexpr int16_t kClockX = 0;
constexpr int16_t kClockY = 0;
constexpr int16_t kClockW = 496;
constexpr int16_t kClockH = 150;
constexpr int16_t kDotPitch = 15;
constexpr int16_t kDotRadius = 6;
constexpr int16_t kDotsX = 10;
constexpr int16_t kDotsY = 8;
constexpr int16_t kDateBaselineY = 140;
constexpr int16_t kDateMarginX = 12;
constexpr int16_t kDateGapX = 12;

// Left column dividers
constexpr int16_t kLeftW = 500;
constexpr int16_t kRuleClockY = 150;
constexpr int16_t kRuleWeatherY = 272;
constexpr int16_t kRuleBottomY = 383;

// Current weather zone
constexpr int16_t kIconCx = 62;
constexpr int16_t kIconCy = 211;
constexpr int16_t kIconSize = 118;
constexpr int16_t kTextX = 125;
constexpr int16_t kTempY = 184;
constexpr int16_t kCondX = 290;
constexpr int16_t kRangeY = 222;
constexpr int16_t kBadgeX = 290;
constexpr int16_t kBadgeH = 28;
constexpr int16_t kBadgePadX = 10;
constexpr int16_t kBadgeRadius = 8;
constexpr int16_t kDetailY = 254;
constexpr int16_t kDetailLine1Y = 245;
constexpr int16_t kDetailLine2Y = 261;
constexpr int16_t kZoneMarginX = 8;

// Forecast zone
constexpr int16_t kFcastDays = 4;
constexpr int16_t kFcastColW = kLeftW / kFcastDays;
constexpr int16_t kFcastDayY = 289;
constexpr int16_t kFcastIconCy = 331;
constexpr int16_t kFcastIconSize = 66;
constexpr int16_t kFcastRangeY = 368;

// Right column
constexpr int16_t kDividerX = 501;
constexpr int16_t kRightX = 508;
constexpr int16_t kRightW = 288;
constexpr int16_t kCalendarY = 0;
constexpr int16_t kBatteryRightPad = 8;
constexpr int16_t kRuleStocksY = 252;
constexpr int16_t kStocksY = 256;
constexpr int16_t kStocksH = 125;

// Bottom graph
constexpr int16_t kGraphY = 385;
constexpr int16_t kGraphH = 95;

int16_t widthOf(Seeed_GFX &gfx, const GFXfont &font, const char *text) {
  return TextUtil::textWidth(gfx, font, text);
}

// First font whose text fits maxW, else the last one.
const GFXfont &fitFont(Seeed_GFX &gfx, const char *text, int16_t maxW,
                       const GFXfont &big, const GFXfont &small) {
  return widthOf(gfx, big, text) <= maxW ? big : small;
}

void drawCentered(Seeed_GFX &gfx, const GFXfont &font, const char *text,
                  int16_t cx, int16_t cy) {
  drawText(gfx, font, text, cx, cy, MC_DATUM);
}

void drawClockContents(Seeed_GFX &gfx, const tm &now) {
  DotClock::draw(gfx, kClockX + kDotsX, kClockY + kDotsY, kDotPitch,
                 kDotRadius, now.tm_hour, now.tm_min);

  char date[48];
  snprintf(date, sizeof(date), "%d %s %d", now.tm_mday,
           Strings::monthLong(now.tm_mon), Strings::displayYear(now.tm_year));
  const char *weekday = Strings::weekdayLong(now.tm_wday);
  const int16_t maxW = kClockW - 2 * kDateMarginX - kDateGapX;
  const int16_t weekdayBigW = widthOf(gfx, Fonts::bold18(), weekday);
  // Shrink the date first; the weekday stays large while it can.
  const bool bothBig = weekdayBigW + widthOf(gfx, Fonts::sans18(), date) <= maxW;
  const bool weekdayBig =
      bothBig || weekdayBigW + widthOf(gfx, Fonts::sans12(), date) <= maxW;
  const GFXfont &weekdayFont = weekdayBig ? Fonts::bold18() : Fonts::bold12();
  const GFXfont &dateFont = bothBig ? Fonts::sans18() : Fonts::sans12();
  drawText(gfx, weekdayFont, weekday, kClockX + kDateMarginX,
           kClockY + kDateBaselineY, L_BASELINE);
  drawText(gfx, dateFont, date, kClockX + kClockW - kDateMarginX,
           kClockY + kDateBaselineY, R_BASELINE);
}

void drawAqiBadge(Seeed_GFX &gfx, const WeatherData &wx) {
  const int16_t maxTextW = kLeftW - kBadgeX - kZoneMarginX - 2 * kBadgePadX;
  char buf[64];
  snprintf(buf, sizeof(buf), "AQI %u %s", static_cast<unsigned>(wx.usAqi),
           Strings::aqiLevel(wx.usAqi));
  if (widthOf(gfx, Fonts::bold9(), buf) > maxTextW) {
    snprintf(buf, sizeof(buf), "AQI %u", static_cast<unsigned>(wx.usAqi));
  }
  const int16_t badgeW = widthOf(gfx, Fonts::bold9(), buf) + 2 * kBadgePadX;
  gfx.fillRoundRect(kBadgeX, kRangeY - kBadgeH / 2, badgeW, kBadgeH,
                    kBadgeRadius, TFT_BLACK);
  drawText(gfx, Fonts::bold9(), buf, kBadgeX + kBadgePadX, kRangeY, ML_DATUM,
           TFT_WHITE);
}

void drawWeatherDetail(Seeed_GFX &gfx, const WeatherData &wx) {
  const int16_t maxW = kLeftW - kTextX - kZoneMarginX;
  char humidity[48];
  char wind[96];
  snprintf(humidity, sizeof(humidity), "%s %u%%", Strings::humidity(),
           static_cast<unsigned>(wx.humidity));
  snprintf(wind, sizeof(wind), "%s %s %.0f %s", Strings::wind(),
           Strings::compass(wx.windDirDeg), wx.windKmh, Strings::windUnit());

  char line[160];
  snprintf(line, sizeof(line), "%s   %s", humidity, wind);
  if (widthOf(gfx, Fonts::sans9(), line) <= maxW) {
    drawText(gfx, Fonts::sans9(), line, kTextX, kDetailY, ML_DATUM);
    return;
  }
  drawText(gfx, Fonts::sans9(), humidity, kTextX, kDetailLine1Y, ML_DATUM);
  drawText(gfx, Fonts::sans9(), wind, kTextX, kDetailLine2Y, ML_DATUM);
}

void drawCurrentWeather(Seeed_GFX &gfx, const WeatherData &wx) {
  char buf[48];
  if (!wx.valid) {
    drawCentered(gfx, Fonts::sans12(), Strings::weatherUnavailable(),
                 kLeftW / 2, (kRuleClockY + kRuleWeatherY) / 2);
    return;
  }

  WeatherIcon::draw(gfx, kIconCx, kIconCy, kIconSize, wx.weatherCode, wx.isDay);

  snprintf(buf, sizeof(buf), "%.0f", wx.tempC);
  drawDegreeText(gfx, Fonts::bold24(), buf, "C", kTextX, kTempY,
                 TextUtil::kLeft);
  const char *cond = WeatherIcon::describe(wx.weatherCode);
  drawText(gfx,
           fitFont(gfx, cond, kLeftW - kCondX - kZoneMarginX, Fonts::bold12(),
                   Fonts::bold9()),
           cond, kCondX, kTempY, ML_DATUM);

  snprintf(buf, sizeof(buf), "%.0f ~ %.0f", wx.daily[0].tempMinC,
           wx.daily[0].tempMaxC);
  drawDegreeText(gfx, Fonts::sans12(), buf, "C", kTextX, kRangeY,
                 TextUtil::kLeft);

  if (wx.aqiValid) {
    drawAqiBadge(gfx, wx);
  }
  drawWeatherDetail(gfx, wx);
}

void drawForecast(Seeed_GFX &gfx, const WeatherData &wx) {
  if (!wx.valid) {
    drawCentered(gfx, Fonts::sans12(), Strings::forecastUnavailable(),
                 kLeftW / 2, (kRuleWeatherY + kRuleBottomY) / 2);
    return;
  }
  char buf[32];
  for (int i = 0; i < kFcastDays; ++i) {
    const DailyForecast &d = wx.daily[i + 1];
    const int16_t cx = i * kFcastColW + kFcastColW / 2;
    tm t;
    const time_t at = d.date;
    if (localtime_r(&at, &t)) {
      snprintf(buf, sizeof(buf), "%s %d", Strings::weekdayShort(t.tm_wday),
               t.tm_mday);
    } else {
      snprintf(buf, sizeof(buf), "--");
    }
    drawCentered(gfx,
                 fitFont(gfx, buf, kFcastColW - 2 * kZoneMarginX,
                         Fonts::bold12(), Fonts::bold9()),
                 buf, cx, kFcastDayY);
    WeatherIcon::draw(gfx, cx, kFcastIconCy, kFcastIconSize, d.weatherCode,
                      true);
    snprintf(buf, sizeof(buf), "%.0f~%.0f", d.tempMinC, d.tempMaxC);
    drawDegreeText(gfx, Fonts::sans12(), buf, nullptr, cx, kFcastRangeY,
                   TextUtil::kCenter);
  }
}

void drawCalendarAndBattery(Seeed_GFX &gfx, const DashboardData &data,
                            const tm &now) {
  MonthCalendar::draw(gfx, kRightX, kCalendarY, kRightW, now);
  const int16_t midY = kCalendarY + MonthCalendar::kHeaderH / 2;
  BatteryGauge::draw(gfx, kRightX + kRightW - kBatteryRightPad, midY,
                     data.batteryPercent, TFT_WHITE);
}

} // namespace

namespace DashboardPage {

Rect clockRect() { return {kClockX, kClockY, kClockW, kClockH}; }

void renderClock(Seeed_GFX &gfx, const tm &now) {
  gfx.fillRect(kClockX, kClockY, kClockW, kClockH, TFT_WHITE);
  drawClockContents(gfx, now);
}

void render(Seeed_GFX &gfx, const DashboardData &data, const tm &now) {
  gfx.fillScreen(TFT_WHITE);

  drawClockContents(gfx, now);
  gfx.drawFastHLine(0, kRuleClockY, kLeftW, TFT_BLACK);
  drawCurrentWeather(gfx, data.weather);
  gfx.drawFastHLine(0, kRuleWeatherY, kLeftW, TFT_BLACK);
  drawForecast(gfx, data.weather);

  gfx.drawFastVLine(kDividerX, 0, kRuleBottomY, TFT_BLACK);
  drawCalendarAndBattery(gfx, data, now);
  gfx.drawFastHLine(kRightX, kRuleStocksY, kRightW, TFT_BLACK);
  StockList::draw(gfx, kRightX, kStocksY, kRightW, kStocksH, data.stocks,
                  data.wifiOk);

  gfx.drawFastHLine(0, kRuleBottomY, gfx.width(), TFT_BLACK);
  if (data.weather.valid) {
    TempGraph::draw(gfx, 0, kGraphY, gfx.width(), kGraphH, data.weather.hourly,
                    kHourlyPoints);
  } else {
    drawCentered(gfx, Fonts::sans12(), Strings::weatherUnavailable(),
                 gfx.width() / 2, kGraphY + kGraphH / 2);
  }
}

} // namespace DashboardPage
