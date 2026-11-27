#include "net/HttpsJson.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "config/AppConfig.h"
#include "net/RootCerts.h"

namespace HttpsJson {

bool get(const char *url, const JsonDocument &filter, JsonDocument &doc, const char *userAgent) {
  WiFiClientSecure client;
  client.setCACert(kRootCerts);
  client.setTimeout(AppConfig::kHttpTimeoutMs);

  HTTPClient http;
  http.setConnectTimeout(AppConfig::kHttpTimeoutMs);
  http.setTimeout(AppConfig::kHttpTimeoutMs);
  if (!http.begin(client, url)) {
    log_e("http begin failed: %s", url);
    return false;
  }
  if (userAgent != nullptr) {
    http.addHeader("User-Agent", userAgent);
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    log_w("GET %s -> %d (%s)", url, code, HTTPClient::errorToString(code).c_str());
    http.end();
    return false;
  }

  // Parsing straight off the TLS stream truncated bodies intermittently.
  const String body = http.getString();
  http.end();
  const DeserializationError err =
      deserializeJson(doc, body, DeserializationOption::Filter(filter));
  if (err) {
    log_w("json parse failed: %s", err.c_str());
    return false;
  }
  return true;
}

} // namespace HttpsJson
