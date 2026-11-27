#include "app/App.h"

#include <Preferences.h>

#include "config/AppConfig.h"
#include "hal/Display/ScreenDump.h"
#include "net/WifiLink.h"
#include "ui/i18n/Lang.h"
#include "ui/pages/DashboardPage.h"
#include "version.h"

namespace {

constexpr uint32_t kNetTaskStack = 16384;
constexpr uint32_t kDisplayTaskStack = 8192;
constexpr UBaseType_t kTaskPriority = 1;
constexpr uint32_t kTickMs = 10;
constexpr uint32_t kRetryMs = 60UL * 1000UL;
// Wake just after the minute rolls over so tm_min has changed.
constexpr uint32_t kMinuteSlackMs = 300;

constexpr uint32_t kEvtFull = 1u << 0;
constexpr uint32_t kEvtDump = 1u << 1;
constexpr uint32_t kEvtData = 1u << 2;
constexpr uint32_t kEvtLang = 1u << 3;

constexpr const char *kPrefsNamespace = "ui";
constexpr const char *kPrefsLangKey = "lang";

Lang loadLanguage() {
  Preferences prefs;
  // Read-write so the first boot creates the namespace instead of logging NOT_FOUND.
  prefs.begin(kPrefsNamespace, false);
  const uint8_t v = prefs.getUChar(kPrefsLangKey, static_cast<uint8_t>(Lang::Th));
  prefs.end();
  return v == static_cast<uint8_t>(Lang::En) ? Lang::En : Lang::Th;
}

void saveLanguage(Lang lang) {
  Preferences prefs;
  prefs.begin(kPrefsNamespace, false);
  prefs.putUChar(kPrefsLangKey, static_cast<uint8_t>(lang));
  prefs.end();
}

} // namespace

bool App::localNow(tm &out) const {
  const time_t t = time(nullptr);
  localtime_r(&t, &out);
  return timeSynced_;
}

TickType_t App::ticksToNextMinute() const {
  tm now;
  if (!localNow(now)) {
    return pdMS_TO_TICKS(kRetryMs);
  }
  return pdMS_TO_TICKS((60 - now.tm_sec) * 1000UL + kMinuteSlackMs);
}

App &App::instance() {
  static App app;
  return app;
}

void App::begin() {
  Serial.begin(115200);
  log_i("EE04 dashboard v%s", FIRMWARE_VERSION);

  buttons_.begin();
  battery_.begin(AppConfig::kBatteryCalScale);
  dataMutex_ = xSemaphoreCreateMutex();
  Language::set(loadLanguage());

  if (!display_.begin()) {
    log_e("display init failed: %s", display_.gfx().lastResult().message);
    return;
  }

  xTaskCreate(displayTask, "display", kDisplayTaskStack, this, kTaskPriority,
              &displayTask_);
  xTaskCreate(netTask, "net", kNetTaskStack, this, kTaskPriority, &netTask_);
}

void App::tick() {
  const uint8_t pressed = buttons_.pollPressed();
  if ((pressed & Buttons::kKey0) && netTask_) {
    xTaskNotifyGive(netTask_);
  }
  if ((pressed & Buttons::kKey1) && displayTask_) {
    xTaskNotify(displayTask_, kEvtFull, eSetBits);
  }
  if ((pressed & Buttons::kKey2) && displayTask_) {
    xTaskNotify(displayTask_, kEvtLang, eSetBits);
  }
#ifdef DEBUG
  pollSerial();
#endif
  delay(kTickMs);
}

void App::pollSerial() {
  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == 'S' && displayTask_) {
      xTaskNotify(displayTask_, kEvtDump, eSetBits);
    } else if (c == 'F' && netTask_) {
      xTaskNotifyGive(netTask_);
    } else if (c == 'L' && displayTask_) {
      xTaskNotify(displayTask_, kEvtLang, eSetBits);
    }
  }
}

void App::netTask(void *arg) { static_cast<App *>(arg)->runNet(); }

void App::displayTask(void *arg) { static_cast<App *>(arg)->runDisplay(); }

void App::runNet() {
  for (;;) {
    fetchAll();
    xTaskNotify(displayTask_, kEvtData, eSetBits);

    // Nothing is drawn until time syncs, so retry that quickly too.
    const bool ok = data_.wifiOk && data_.weather.valid && timeSynced_;
    const uint32_t waitMs = ok ? AppConfig::kDataRefreshMs : kRetryMs;
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(waitMs));
  }
}

void App::fetchAll() {
  xSemaphoreTake(dataMutex_, portMAX_DELAY);
  WeatherData weather = data_.weather;
  StocksData stocks = data_.stocks;
  xSemaphoreGive(dataMutex_);

  radioOn_ = true;
  const bool wifiOk = WifiLink::connect(AppConfig::kWifiConnectTimeoutMs);
  if (wifiOk) {
    // Resync every fetch; WiFi is off between fetches so SNTP cannot run alone.
    if (WifiLink::syncTime(AppConfig::kNtpSyncTimeoutMs)) {
      timeSynced_ = true;
    }
    if (weather_.fetch(weather)) {
      log_i("weather ok: %.1f C code %d, aqi %u", weather.tempC,
            weather.weatherCode, weather.usAqi);
    }
    if (stocks_.fetch(stocks)) {
      log_i("stocks ok");
    }
  }
  WifiLink::disconnect();
  radioOn_ = false;

  xSemaphoreTake(dataMutex_, portMAX_DELAY);
  data_.weather = weather;
  data_.stocks = stocks;
  data_.wifiOk = wifiOk;
  xSemaphoreGive(dataMutex_);

  log_i("fetch done, free heap %lu, min %lu",
        static_cast<unsigned long>(ESP.getFreeHeap()),
        static_cast<unsigned long>(ESP.getMinFreeHeap()));
}

void App::runDisplay() {
  int lastMinute = -1;
  int lastHour = -1;

  for (;;) {
    uint32_t events = 0;
    xTaskNotifyWait(0, UINT32_MAX, &events, ticksToNextMinute());

    if (events & kEvtDump) {
      dumpScreen(display_.gfx(), Serial);
    }
    // Switched here so a render never sees the language change mid-page.
    if (events & kEvtLang) {
      const Lang next = Language::current() == Lang::Th ? Lang::En : Lang::Th;
      Language::set(next);
      saveLanguage(next);
      events |= kEvtFull;
      log_i("language -> %s", next == Lang::Th ? "th" : "en");
    }

    tm now;
    if (!localNow(now)) {
      continue;
    }
    // Before any panel refresh, so the reading is not under refresh load.
    sampleBattery();
    // Full refresh only hourly (and first draw) to clear partial-refresh ghosting.
    if ((events & kEvtFull) || now.tm_hour != lastHour) {
      drawPage(now, true);
    } else if (events & kEvtData) {
      drawPage(now, false);
    } else if (now.tm_min != lastMinute) {
      drawClock(now);
    }
    lastMinute = now.tm_min;
    lastHour = now.tm_hour;
  }
}

void App::sampleBattery() {
  // WiFi TX sags the cell by tens of mV, which reads as a lower charge.
  if (radioOn_) {
    return;
  }
  const uint32_t mv = battery_.readMillivolts();
  if (!radioOn_) {
    batteryLevel_.add(mv);
  }
}

void App::drawPage(const tm &now, bool fullRefresh) {
  const int8_t percent =
      batteryLevel_.valid() ? static_cast<int8_t>(batteryLevel_.percent()) : -1;

  xSemaphoreTake(dataMutex_, portMAX_DELAY);
  data_.batteryPercent = percent;
  const DashboardData snapshot = data_;
  xSemaphoreGive(dataMutex_);

  Seeed_GFX &gfx = display_.gfx();
  DashboardPage::render(gfx, snapshot, now);
  const uint32_t startMs = millis();
  const GfxResult result =
      fullRefresh ? gfx.refresh()
                  : gfx.refreshPartial(0, 0, gfx.width(), gfx.height());
  if (!result) {
    log_e("page refresh failed: %s", result.message);
    return;
  }
  log_i("page %s refresh ok in %lu ms, battery %lu mV %d%%",
        fullRefresh ? "full" : "partial",
        static_cast<unsigned long>(millis() - startMs),
        static_cast<unsigned long>(batteryLevel_.millivolts()), percent);
}

void App::drawClock(const tm &now) {
  const DashboardPage::Rect r = DashboardPage::clockRect();
  DashboardPage::renderClock(display_.gfx(), now);
  const uint32_t startMs = millis();
  const GfxResult result = display_.gfx().refreshPartial(r.x, r.y, r.w, r.h);
  if (!result) {
    log_e("clock refresh failed: %s", result.message);
    return;
  }
  log_i("clock %02d:%02d partial ok in %lu ms", now.tm_hour, now.tm_min,
        static_cast<unsigned long>(millis() - startMs));
}
