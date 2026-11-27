#pragma once

#include <ArduinoJson.h>

namespace HttpsJson {
// Verified HTTPS GET; parses the body through `filter`. `userAgent` may be null.
bool get(const char *url, const JsonDocument &filter, JsonDocument &doc, const char *userAgent = nullptr);
} // namespace HttpsJson
