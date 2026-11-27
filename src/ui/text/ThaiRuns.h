#pragma once

#include <Arduino.h>

// A run starts at a codepoint U+0E00..U+0E7F and continues while the next
// codepoint is U+0E00..U+0E7F, '.' or '/'. scripts/gen_thai_assets.py uses the same rule.
struct ThaiRun {
  const char *text; // UTF-8, exactly one run
  uint8_t font;     // Fonts::Id
  uint16_t width;   // bitmap size in px
  uint16_t height;
  int16_t xOffset;  // bitmap left relative to pen x
  int16_t yOffset;  // bitmap top relative to baseline, negative is above
  uint16_t advance; // pen advance in px
  const uint8_t *bits; // 1 bpp, MSB first, rows padded to whole bytes
};

namespace ThaiRuns {
// nullptr when the run was not generated for that font.
const ThaiRun *find(uint8_t font, const char *text, size_t len);
} // namespace ThaiRuns
