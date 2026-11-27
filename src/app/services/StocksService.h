#pragma once

#include "app/model/DashboardData.h"

// Yahoo Finance chart API for AppConfig::kStocks. Needs WiFi.
class StocksService {
public:
  // Updates each quote independently; returns true if at least one succeeded.
  bool fetch(StocksData &out);
};
