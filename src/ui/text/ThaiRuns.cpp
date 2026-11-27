#include "ui/text/ThaiRuns.h"

#include <cstring>

extern const ThaiRun kThaiRuns[];
extern const size_t kThaiRunCount;

namespace ThaiRuns {

const ThaiRun *find(uint8_t font, const char *text, size_t len) {
  for (size_t i = 0; i < kThaiRunCount; ++i) {
    const ThaiRun &r = kThaiRuns[i];
    if (r.font == font && strncmp(r.text, text, len) == 0 && r.text[len] == '\0') {
      return &r;
    }
  }
  return nullptr;
}

} // namespace ThaiRuns
