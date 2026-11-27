#include "hal/Display/ScreenDump.h"

void dumpScreen(Seeed_GFX &gfx, Print &out) {
  static const char kHex[] = "0123456789ABCDEF";
  const int32_t w = gfx.width();
  const int32_t h = gfx.height();
  char line[2 * 128 + 2];

  out.printf("=====SCREEN BEGIN %ld %ld=====\n", static_cast<long>(w),
             static_cast<long>(h));
  for (int32_t y = 0; y < h; ++y) {
    size_t n = 0;
    for (int32_t x0 = 0; x0 < w; x0 += 8) {
      uint8_t bits = 0;
      for (int32_t b = 0; b < 8 && x0 + b < w; ++b) {
        if (gfx.readPixel(x0 + b, y) == TFT_BLACK) {
          bits |= 0x80 >> b;
        }
      }
      line[n++] = kHex[bits >> 4];
      line[n++] = kHex[bits & 0x0F];
      if (n >= sizeof(line) - 2) {
        out.write(reinterpret_cast<const uint8_t *>(line), n);
        n = 0;
      }
    }
    line[n++] = '\n';
    out.write(reinterpret_cast<const uint8_t *>(line), n);
  }
  out.print("=====SCREEN END=====\n");
}
