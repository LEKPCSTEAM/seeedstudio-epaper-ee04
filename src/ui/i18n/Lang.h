#pragma once

#include <Arduino.h>

enum class Lang : uint8_t { En = 0, Th = 1 };

namespace Language {
Lang current();
void set(Lang lang);
} // namespace Language
