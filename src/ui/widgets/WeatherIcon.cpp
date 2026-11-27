#include "ui/widgets/WeatherIcon.h"

#include <cmath>

#include "ui/i18n/Strings.h"
#include "ui/widgets/TextUtil.h"

namespace WeatherIcon {

namespace {

void cloudShape(Seeed_GFX &gfx, float cx, float cy, float u, float shrink,
                uint16_t color) {
  auto rad = [shrink](float r) {
    return static_cast<int32_t>(std::max(1.0f, r - shrink));
  };
  gfx.fillCircle(cx - 24 * u, cy + 6 * u, rad(20 * u), color);
  gfx.fillCircle(cx - 2 * u, cy - 6 * u, rad(28 * u), color);
  gfx.fillCircle(cx + 26 * u, cy + 8 * u, rad(18 * u), color);
  gfx.fillRect(cx - 24 * u, cy + 6 * u, 50 * u,
               std::max(1.0f, 20 * u - shrink), color);
}

// Cloud outline spanning width w; bottom edge sits at cy + 0.26 w.
void drawCloud(Seeed_GFX &gfx, float cx, float cy, float w, float t) {
  const float u = w / 100.0f;
  cloudShape(gfx, cx, cy, u, 0, TFT_BLACK);
  cloudShape(gfx, cx, cy, u, t, TFT_WHITE);
}

void drawSun(Seeed_GFX &gfx, float cx, float cy, float r, float t,
             float rayFrom, float rayTo) {
  gfx.fillCircle(cx, cy, r, TFT_BLACK);
  gfx.fillCircle(cx, cy, r - t, TFT_WHITE);
  for (int k = 0; k < 8; ++k) {
    const float a = k * PI / 4.0f;
    TextUtil::thickLine(gfx, cx + cosf(a) * rayFrom, cy + sinf(a) * rayFrom,
                        cx + cosf(a) * rayTo, cy + sinf(a) * rayTo, t);
  }
}

void drawMoon(Seeed_GFX &gfx, float cx, float cy, float r) {
  gfx.fillCircle(cx, cy, r, TFT_BLACK);
  gfx.fillCircle(cx + 0.5f * r, cy - 0.35f * r, 0.8f * r, TFT_WHITE);
}

void drawBolt(Seeed_GFX &gfx, float cx, float y0, float s) {
  gfx.fillTriangle(cx + 0.06f * s, y0, cx - 0.10f * s, y0 + 0.15f * s,
                   cx + 0.03f * s, y0 + 0.15f * s, TFT_BLACK);
  gfx.fillTriangle(cx - 0.03f * s, y0 + 0.13f * s, cx + 0.10f * s,
                   y0 + 0.13f * s, cx - 0.06f * s, y0 + 0.30f * s, TFT_BLACK);
}

} // namespace

Kind kindFromCode(int16_t c) {
  if (c == 0) return Kind::kClear;
  if (c == 1 || c == 2) return Kind::kPartlyCloudy;
  if (c == 45 || c == 48) return Kind::kFog;
  if (c >= 51 && c <= 57) return Kind::kDrizzle;
  if ((c >= 61 && c <= 67) || (c >= 80 && c <= 82)) return Kind::kRain;
  if ((c >= 71 && c <= 77) || c == 85 || c == 86) return Kind::kSnow;
  if (c >= 95 && c <= 99) return Kind::kThunder;
  return Kind::kOvercast;
}

const char *describe(int16_t c) { return Strings::condition(c); }

void draw(Seeed_GFX &gfx, int16_t cx, int16_t cy, int16_t size,
          int16_t wmoCode, bool isDay) {
  const float s = size;
  const float t = std::max(2.0f, s / 30.0f);
  const Kind kind = kindFromCode(wmoCode);

  if (kind == Kind::kClear) {
    if (isDay) {
      drawSun(gfx, cx, cy, 0.2f * s, t, 0.32f * s, 0.48f * s);
    } else {
      drawMoon(gfx, cx, cy, 0.34f * s);
    }
    return;
  }

  if (kind == Kind::kPartlyCloudy) {
    if (isDay) {
      drawSun(gfx, cx - 0.16f * s, cy - 0.16f * s, 0.14f * s, t, 0.21f * s,
              0.31f * s);
    } else {
      drawMoon(gfx, cx - 0.14f * s, cy - 0.16f * s, 0.24f * s);
    }
    drawCloud(gfx, cx + 0.08f * s, cy + 0.06f * s, 0.78f * s, t);
    return;
  }

  const float cloudY = cy - 0.14f * s;
  const float y0 = cy + 0.14f * s;
  const float y1 = cy + 0.42f * s;
  drawCloud(gfx, cx, cloudY, 0.8f * s, t);

  switch (kind) {
  case Kind::kFog:
    TextUtil::thickLine(gfx, cx - 0.32f * s, y0 + 0.05f * s, cx + 0.32f * s,
                        y0 + 0.05f * s, t);
    TextUtil::thickLine(gfx, cx - 0.22f * s, y0 + 0.19f * s, cx + 0.38f * s,
                        y0 + 0.19f * s, t);
    break;
  case Kind::kDrizzle:
    for (int i = -1; i <= 1; ++i) {
      const float x = cx + i * 0.22f * s;
      const float off = (i == 0) ? 0.06f * s : 0;
      TextUtil::thickLine(gfx, x, y0 + off, x - 0.04f * s,
                          y0 + off + 0.12f * s, t);
    }
    break;
  case Kind::kRain:
    for (int i = -1; i <= 1; ++i) {
      const float x = cx + i * 0.22f * s;
      TextUtil::thickLine(gfx, x + 0.03f * s, y0, x - 0.05f * s, y1, t);
    }
    break;
  case Kind::kSnow:
    for (int i = -1; i <= 1; ++i) {
      const float x = cx + i * 0.22f * s;
      const float y = (y0 + y1) / 2 + ((i == 0) ? 0.04f * s : 0);
      const float r = 0.08f * s;
      for (int k = 0; k < 3; ++k) {
        const float a = k * PI / 3.0f;
        TextUtil::thickLine(gfx, x - cosf(a) * r, y - sinf(a) * r,
                            x + cosf(a) * r, y + sinf(a) * r, 2);
      }
    }
    break;
  case Kind::kThunder:
    drawBolt(gfx, cx, y0 - 0.02f * s, s);
    break;
  default:
    break;
  }
}

} // namespace WeatherIcon
