#pragma once

#include "app/model/DashboardData.h"

// Open-Meteo forecast + air quality for AppConfig location. Needs WiFi and synced time.
class WeatherService {
public:
  // Leaves `out` untouched on failure.
  bool fetch(WeatherData &out);
};
