#include "net/WifiLink.h"

#include <WiFi.h>
#include <ctime>
#include <esp_sntp.h>

#include "config/AppConfig.h"
#include "config/Env.h"

namespace WifiLink {

bool connect(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= timeoutMs) {
      log_w("wifi connect timeout, status=%d", static_cast<int>(WiFi.status()));
      return false;
    }
    delay(100);
  }
  log_i("wifi connected, rssi=%d", WiFi.RSSI());
  return true;
}

void disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool isConnected() { return WiFi.status() == WL_CONNECTED; }

bool syncTime(uint32_t timeoutMs) {
  // The RTC survives resets with stale time, so wait for SNTP itself.
  configTzTime(AppConfig::kPosixTz, AppConfig::kNtpServer1, AppConfig::kNtpServer2);
  const uint32_t start = millis();
  while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
    if (millis() - start >= timeoutMs) {
      log_w("ntp sync timeout");
      return false;
    }
    delay(100);
  }
  log_i("time synced: %ld", static_cast<long>(time(nullptr)));
  return true;
}

} // namespace WifiLink
