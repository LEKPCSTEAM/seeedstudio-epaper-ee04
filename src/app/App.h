#pragma once

#include <Arduino.h>
#include <atomic>
#include <ctime>

#include "app/model/DashboardData.h"
#include "app/services/StocksService.h"
#include "app/services/WeatherService.h"
#include "hal/Battery/Battery.h"
#include "hal/Battery/BatteryLevel.h"
#include "hal/Buttons/Buttons.h"
#include "hal/Display/EpaperDisplay.h"

// main.cpp only calls App::instance().begin() / .tick().
class App {
public:
  static App &instance();

  void begin();
  void tick();

private:
  App() = default;
  App(const App &) = delete;
  App &operator=(const App &) = delete;

  static void netTask(void *arg);
  static void displayTask(void *arg);
  void runNet();
  void runDisplay();
  void fetchAll();
  void drawPage(const tm &now, bool fullRefresh);
  void drawClock(const tm &now);
  void sampleBattery();
  void pollSerial();
  bool localNow(tm &out) const;
  TickType_t ticksToNextMinute() const;

  EpaperDisplay display_;
  Buttons buttons_;
  Battery battery_;
  BatteryLevel batteryLevel_; // display task only
  WeatherService weather_;
  StocksService stocks_;

  SemaphoreHandle_t dataMutex_ = nullptr;
  DashboardData data_;
  std::atomic<bool> timeSynced_{false};
  std::atomic<bool> radioOn_{false};

  TaskHandle_t netTask_ = nullptr;
  TaskHandle_t displayTask_ = nullptr;
};
