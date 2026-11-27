#include "app/services/WeatherService.h"

#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "net/HttpsJson.h"

namespace {

bool fetchForecast(WeatherData &w) {
  char url[512];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m,is_day"
           "&hourly=temperature_2m"
           "&daily=weather_code,temperature_2m_max,temperature_2m_min"
           "&timezone=Asia%%2FBangkok&forecast_days=%u&forecast_hours=%u&timeformat=unixtime",
           AppConfig::kLatitude, AppConfig::kLongitude, static_cast<unsigned>(kForecastDays),
           static_cast<unsigned>((kHourlyPoints - 1) * kHourlyStepHours + 1));

  JsonDocument filter;
  filter["current"] = true;
  filter["hourly"] = true;
  filter["daily"] = true;

  JsonDocument doc;
  if (!HttpsJson::get(url, filter, doc)) {
    return false;
  }

  JsonObjectConst cur = doc["current"];
  JsonArrayConst hTime = doc["hourly"]["time"];
  JsonArrayConst hTemp = doc["hourly"]["temperature_2m"];
  JsonArrayConst dTime = doc["daily"]["time"];
  JsonArrayConst dCode = doc["daily"]["weather_code"];
  JsonArrayConst dMax = doc["daily"]["temperature_2m_max"];
  JsonArrayConst dMin = doc["daily"]["temperature_2m_min"];

  if (cur["temperature_2m"].isNull() || dTime.size() < kForecastDays || dCode.size() < kForecastDays ||
      dMax.size() < kForecastDays || dMin.size() < kForecastDays) {
    log_w("forecast json missing fields");
    return false;
  }
  const size_t lastHourly = static_cast<size_t>(kHourlyPoints - 1) * kHourlyStepHours;
  if (hTime.size() <= lastHourly || hTemp.size() <= lastHourly) {
    log_w("forecast hourly too short");
    return false;
  }

  w.tempC = cur["temperature_2m"];
  w.humidity = cur["relative_humidity_2m"] | 0;
  w.weatherCode = cur["weather_code"] | -1;
  w.windKmh = cur["wind_speed_10m"] | 0.0f;
  w.windDirDeg = cur["wind_direction_10m"] | 0;
  w.isDay = (cur["is_day"] | 1) != 0;

  for (uint8_t i = 0; i < kForecastDays; i++) {
    w.daily[i].date = dTime[i].as<time_t>();
    w.daily[i].weatherCode = dCode[i] | -1;
    w.daily[i].tempMaxC = dMax[i];
    w.daily[i].tempMinC = dMin[i];
  }
  for (uint8_t i = 0; i < kHourlyPoints; i++) {
    const size_t idx = static_cast<size_t>(i) * kHourlyStepHours;
    w.hourly[i].time = hTime[idx].as<time_t>();
    w.hourly[i].tempC = hTemp[idx];
  }
  return true;
}

bool fetchAirQuality(WeatherData &w) {
  char url[256];
  snprintf(url, sizeof(url),
           "https://air-quality-api.open-meteo.com/v1/air-quality?latitude=%.4f&longitude=%.4f"
           "&current=us_aqi,pm2_5&timezone=Asia%%2FBangkok",
           AppConfig::kLatitude, AppConfig::kLongitude);

  JsonDocument filter;
  filter["current"] = true;

  JsonDocument doc;
  if (!HttpsJson::get(url, filter, doc)) {
    return false;
  }
  JsonObjectConst cur = doc["current"];
  if (cur["us_aqi"].isNull() || cur["pm2_5"].isNull()) {
    log_w("air quality json missing fields");
    return false;
  }
  w.usAqi = cur["us_aqi"];
  w.pm25 = cur["pm2_5"];
  return true;
}

} // namespace

bool WeatherService::fetch(WeatherData &out) {
  WeatherData w = out;
  if (!fetchForecast(w)) {
    return false;
  }
  w.aqiValid = fetchAirQuality(w);
  w.valid = true;
  w.updatedAt = time(nullptr);
  out = w;
  log_i("weather ok: %.1fC code=%d aqi=%d", w.tempC, w.weatherCode, w.aqiValid ? w.usAqi : -1);
  return true;
}
