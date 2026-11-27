#pragma once

#include <Arduino.h>

namespace AppConfig {

// Bangkok
constexpr float kLatitude = 13.7563f;
constexpr float kLongitude = 100.5018f;
constexpr const char *kTimezoneName = "Asia/Bangkok";
// POSIX TZ: ICT is UTC+7, no DST.
constexpr const char *kPosixTz = "ICT-7";
constexpr const char *kNtpServer1 = "pool.ntp.org";
constexpr const char *kNtpServer2 = "time.google.com";

struct StockSymbol {
  const char *symbol;
  const char *label;
};

constexpr StockSymbol kStocks[] = {
    {"^SET.BK", "SET"},
    {"AAPL", "AAPL"},
    {"NVDA", "NVDA"},
    {"^IXIC", "NASDAQ"},
};

constexpr uint32_t kDataRefreshMs = 15UL * 60UL * 1000UL;
constexpr uint32_t kWifiConnectTimeoutMs = 20000;
constexpr uint32_t kNtpSyncTimeoutMs = 15000;
constexpr uint32_t kHttpTimeoutMs = 15000;

// Per-board ADC trim: multimeter VBAT / logged battery mV.
constexpr float kBatteryCalScale = 1.0f;

} // namespace AppConfig
