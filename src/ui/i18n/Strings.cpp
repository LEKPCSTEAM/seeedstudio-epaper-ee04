#include "ui/i18n/Strings.h"

#include "ui/i18n/Lang.h"

namespace {

bool th() { return Language::current() == Lang::Th; }

const char *pick(const char *en, const char *thai) { return th() ? thai : en; }

constexpr const char *kWeekdayLongEn[7] = {"Sunday",   "Monday", "Tuesday",
                                           "Wednesday", "Thursday", "Friday",
                                           "Saturday"};
constexpr const char *kWeekdayLongTh[7] = {
    "วันอาทิตย์", "วันจันทร์", "วันอังคาร", "วันพุธ",
    "วันพฤหัสบดี", "วันศุกร์", "วันเสาร์"};
constexpr const char *kWeekdayShortEn[7] = {"Sun", "Mon", "Tue", "Wed",
                                            "Thu", "Fri", "Sat"};
constexpr const char *kWeekdayShortTh[7] = {"อา.", "จ.", "อ.", "พ.",
                                            "พฤ.", "ศ.", "ส."};
constexpr const char *kWeekdayMinTh[7] = {"อา", "จ", "อ", "พ", "พฤ", "ศ", "ส"};
constexpr const char *kMonthEn[12] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December"};
constexpr const char *kMonthTh[12] = {
    "มกราคม",  "กุมภาพันธ์", "มีนาคม",   "เมษายน",  "พฤษภาคม",  "มิถุนายน",
    "กรกฎาคม", "สิงหาคม",   "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"};
constexpr const char *kCompassEn[8] = {"N", "NE", "E", "SE",
                                       "S", "SW", "W", "NW"};
constexpr const char *kCompassTh[8] = {
    "เหนือ", "ตะวันออกเฉียงเหนือ", "ตะวันออก", "ตะวันออกเฉียงใต้",
    "ใต้",   "ตะวันตกเฉียงใต้",   "ตะวันตก",  "ตะวันตกเฉียงเหนือ"};
// Thai Buddhist era = CE + 543.
constexpr int kBuddhistEraOffset = 543;

int clampIndex(int v, int maxIndex) { return v < 0 ? 0 : (v > maxIndex ? maxIndex : v); }

} // namespace

namespace Strings {

const char *weekdayLong(int wday) {
  const int i = clampIndex(wday, 6);
  return pick(kWeekdayLongEn[i], kWeekdayLongTh[i]);
}

const char *weekdayShort(int wday) {
  const int i = clampIndex(wday, 6);
  return pick(kWeekdayShortEn[i], kWeekdayShortTh[i]);
}

const char *weekdayMin(int wday) {
  const int i = clampIndex(wday, 6);
  return pick(kWeekdayShortEn[i], kWeekdayMinTh[i]);
}

const char *monthLong(int mon) {
  const int i = clampIndex(mon, 11);
  return pick(kMonthEn[i], kMonthTh[i]);
}

int displayYear(int tmYear) {
  return tmYear + 1900 + (th() ? kBuddhistEraOffset : 0);
}

const char *condition(int16_t code) {
  switch (code) {
  case 0: return pick("Clear", "ท้องฟ้าแจ่มใส");
  case 1: return pick("Mainly clear", "ท้องฟ้าโปร่ง");
  case 2: return pick("Partly cloudy", "มีเมฆบางส่วน");
  case 3: return pick("Overcast", "เมฆมาก");
  case 45:
  case 48: return pick("Fog", "หมอก");
  case 65:
  case 82: return pick("Heavy rain", "ฝนตกหนัก");
  case 80:
  case 81: return pick("Showers", "ฝนตกเป็นช่วง");
  default: break;
  }
  if (code >= 51 && code <= 57) return pick("Drizzle", "ฝนปรอย");
  if (code >= 61 && code <= 67) return pick("Rain", "ฝนตก");
  if ((code >= 71 && code <= 77) || code == 85 || code == 86)
    return pick("Snow", "หิมะ");
  if (code >= 95 && code <= 99) return pick("Thunderstorm", "พายุฝนฟ้าคะนอง");
  return pick("Cloudy", "มีเมฆ");
}

const char *aqiLevel(uint16_t aqi) {
  if (aqi <= 50) return pick("Good", "ดี");
  if (aqi <= 100) return pick("Moderate", "ปานกลาง");
  if (aqi <= 150) return pick("Sensitive", "เริ่มมีผลต่อสุขภาพ");
  if (aqi <= 200) return pick("Unhealthy", "มีผลต่อสุขภาพ");
  if (aqi <= 300) return pick("Very Unhealthy", "มีผลต่อสุขภาพมาก");
  return pick("Hazardous", "อันตราย");
}

const char *compass(uint16_t deg) {
  const int i = ((deg + 22) / 45) % 8;
  return pick(kCompassEn[i], kCompassTh[i]);
}

const char *humidity() { return pick("Humidity", "ความชื้น"); }
const char *wind() { return pick("Wind", "ลม"); }
const char *windUnit() { return pick("km/h", "กม./ชม."); }
const char *markets() { return pick("Markets", "ตลาดหุ้น"); }
const char *updated() { return pick("Updated", "อัปเดต"); }
const char *now() { return pick("Now", "ตอนนี้"); }
const char *noWifi() { return pick("No WiFi", "ไม่มี WiFi"); }
const char *weatherUnavailable() { return pick("Weather unavailable", "ไม่มีข้อมูลอากาศ"); }
const char *forecastUnavailable() { return pick("Forecast unavailable", "ไม่มีข้อมูลพยากรณ์"); }

} // namespace Strings
