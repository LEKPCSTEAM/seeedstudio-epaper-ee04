#pragma once

#include <Arduino.h>

namespace WifiLink {
bool connect(uint32_t timeoutMs);
void disconnect();
bool isConnected();
// Configures AppConfig::kPosixTz and waits for NTP.
bool syncTime(uint32_t timeoutMs);
} // namespace WifiLink
