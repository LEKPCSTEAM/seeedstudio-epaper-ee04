#pragma once

#include <Arduino.h>
#include <ctime>

// Today plus the next four days.
constexpr uint8_t kForecastDays = 5;
// Now through +24 h, one point every 3 h.
constexpr uint8_t kHourlyPoints = 9;
constexpr uint8_t kHourlyStepHours = 3;
constexpr uint8_t kQuoteCount = 4;

struct DailyForecast {
  time_t date = 0; // local midnight
  int16_t weatherCode = -1; // WMO code
  float tempMinC = 0;
  float tempMaxC = 0;
};

struct HourlyTemp {
  time_t time = 0;
  float tempC = 0;
};

struct WeatherData {
  bool valid = false;
  time_t updatedAt = 0;
  float tempC = 0;
  uint8_t humidity = 0;
  int16_t weatherCode = -1; // WMO code
  bool isDay = true;
  float windKmh = 0;
  uint16_t windDirDeg = 0;
  DailyForecast daily[kForecastDays];
  HourlyTemp hourly[kHourlyPoints];
  bool aqiValid = false;
  uint16_t usAqi = 0;
  float pm25 = 0; // ug/m3
};

struct Quote {
  char symbol[16] = {};
  char label[12] = {};
  bool valid = false;
  float price = 0;
  float previousClose = 0;
};

struct StocksData {
  time_t updatedAt = 0;
  Quote quotes[kQuoteCount];
};

struct DashboardData {
  WeatherData weather;
  StocksData stocks;
  // -1 until the first resting sample.
  int8_t batteryPercent = -1;
  // Optimistic until the first fetch attempt reports otherwise.
  bool wifiOk = true;
};
