#pragma once

#include <Arduino.h>

// UI text for Language::current(). Never join two Thai strings without a space:
// Thai is drawn from pre-shaped runs generated from these literals.
namespace Strings {

const char *weekdayLong(int wday);  // "Thursday"
const char *weekdayShort(int wday); // "Thu"
const char *weekdayMin(int wday);   // calendar header
const char *monthLong(int mon);     // tm_mon 0..11
int displayYear(int tmYear);        // Buddhist era in Thai
const char *condition(int16_t wmoCode);
const char *aqiLevel(uint16_t aqi);
const char *compass(uint16_t deg);
const char *humidity();
const char *wind();
const char *windUnit();
const char *markets();
const char *updated();
const char *now();
const char *noWifi();
const char *weatherUnavailable();
const char *forecastUnavailable();

} // namespace Strings
