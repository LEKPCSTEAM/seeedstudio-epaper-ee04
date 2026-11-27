#include "ui/widgets/TextUtil.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "ui/text/ThaiRuns.h"
#include "ui/widgets/Fonts.h"

namespace TextUtil {

namespace {

constexpr int16_t kMissingBoxGap = 2;
constexpr size_t kSegmentMax = 64;

struct Segment {
  const char *p;
  size_t len;
  bool thai;
};

bool isThaiCp(uint32_t cp) { return cp >= 0x0E00 && cp <= 0x0E7F; }

size_t decodeCp(const char *s, size_t left, uint32_t &cp) {
  const uint8_t b0 = static_cast<uint8_t>(s[0]);
  const size_t n = b0 < 0x80 ? 1 : (b0 >> 5) == 6 ? 2 : (b0 >> 4) == 14 ? 3 : (b0 >> 3) == 30 ? 4 : 1;
  if (n > left || (n == 1 && b0 >= 0x80)) {
    cp = 0xFFFD;
    return 1;
  }
  cp = n == 1 ? b0 : b0 & (0xFF >> (n + 1));
  for (size_t i = 1; i < n; ++i) {
    cp = (cp << 6) | (static_cast<uint8_t>(s[i]) & 0x3F);
  }
  return n;
}

bool hasThai(const char *text) {
  for (const char *p = text; *p; ++p) {
    if (static_cast<uint8_t>(*p) == 0xE0) {
      return true;
    }
  }
  return false;
}

template <typename Fn> void forEachSegment(const char *text, Fn fn) {
  const size_t total = strlen(text);
  size_t i = 0;
  while (i < total) {
    uint32_t cp;
    const size_t n = decodeCp(text + i, total - i, cp);
    const bool thai = isThaiCp(cp);
    size_t j = i + n;
    while (j < total) {
      uint32_t next;
      const size_t m = decodeCp(text + j, total - j, next);
      const bool same = thai ? (isThaiCp(next) || next == '.' || next == '/') : !isThaiCp(next);
      if (!same) {
        break;
      }
      j += m;
    }
    fn(Segment{text + i, j - i, thai});
    i = j;
  }
}

// Last char of the string counts offset + width, like the library.
int16_t asciiWidth(const GFXfont &font, const Segment &seg, bool isLast) {
  int16_t w = 0;
  size_t i = 0;
  while (i < seg.len) {
    uint32_t cp;
    i += decodeCp(seg.p + i, seg.len - i, cp);
    if (cp < font.first || cp > font.last) {
      continue;
    }
    const GFXglyph &g = font.glyph[cp - font.first];
    w += (isLast && i >= seg.len) ? g.xOffset + g.width : g.xAdvance;
  }
  return w;
}

int16_t missingWidth(const GFXfont &font) { return font.yAdvance / 2 + kMissingBoxGap; }

void logMissing(const Segment &seg) {
  static char last[kSegmentMax];
  char buf[kSegmentMax];
  const size_t n = std::min(seg.len, kSegmentMax - 1);
  memcpy(buf, seg.p, n);
  buf[n] = '\0';
  if (strcmp(buf, last) != 0) {
    strcpy(last, buf);
    log_w("missing Thai run: %s", buf);
  }
}

int16_t mixedWidth(const GFXfont &font, const char *text) {
  const uint8_t id = Fonts::idOf(font);
  const char *end = text + strlen(text);
  int16_t w = 0;
  forEachSegment(text, [&](const Segment &seg) {
    if (!seg.thai) {
      w += asciiWidth(font, seg, seg.p + seg.len == end);
    } else if (const ThaiRun *run = ThaiRuns::find(id, seg.p, seg.len)) {
      w += run->advance;
    } else {
      w += missingWidth(font);
    }
  });
  return w;
}

// Same ascent/descent scan as Seeed_GFX::setFreeFont.
void freeFontExtents(const GFXfont &font, int16_t &ascent, int16_t &descent) {
  ascent = 0;
  descent = 0;
  for (uint32_t cp = font.first; cp <= font.last; ++cp) {
    const GFXglyph &g = font.glyph[cp - font.first];
    const int16_t ab = -g.yOffset;
    ascent = std::max<int16_t>(ascent, ab);
    descent = std::max<int16_t>(descent, g.height - ab);
  }
}

void drawMixed(Seeed_GFX &gfx, const GFXfont &font, const char *text, int16_t x, int16_t y,
               uint8_t datum, uint16_t color) {
  int16_t ab, bb;
  freeFontExtents(font, ab, bb);
  const int16_t width = mixedWidth(font, text);
  const int16_t height = ab + bb;

  int16_t penX = x;
  switch (datum) {
  case TC_DATUM: case MC_DATUM: case BC_DATUM: case C_BASELINE: penX -= width / 2; break;
  case TR_DATUM: case MR_DATUM: case BR_DATUM: case R_BASELINE: penX -= width; break;
  default: break;
  }
  int16_t top = y;
  switch (datum) {
  case ML_DATUM: case MC_DATUM: case MR_DATUM: top -= height / 2; break;
  case BL_DATUM: case BC_DATUM: case BR_DATUM: top -= height; break;
  case L_BASELINE: case C_BASELINE: case R_BASELINE: top -= ab; break;
  default: break;
  }
  const int16_t baseline = top + ab;

  const uint8_t id = Fonts::idOf(font);
  const char *end = text + strlen(text);
  gfx.setFreeFont(&font);
  gfx.setTextColor(color);
  gfx.setTextDatum(L_BASELINE);
  forEachSegment(text, [&](const Segment &seg) {
    if (!seg.thai) {
      char buf[kSegmentMax];
      const size_t n = std::min(seg.len, kSegmentMax - 1);
      memcpy(buf, seg.p, n);
      buf[n] = '\0';
      gfx.drawString(buf, penX, baseline);
      penX += asciiWidth(font, seg, seg.p + seg.len == end);
    } else if (const ThaiRun *run = ThaiRuns::find(id, seg.p, seg.len)) {
      gfx.drawBitmap(penX + run->xOffset, baseline + run->yOffset, run->bits, run->width,
                     run->height, color);
      penX += run->advance;
    } else {
      logMissing(seg);
      gfx.drawRect(penX, baseline - ab, font.yAdvance / 2, ab, color);
      penX += missingWidth(font);
    }
  });
}

} // namespace

int16_t textWidth(Seeed_GFX &gfx, const GFXfont &font, const char *text) {
  if (!hasThai(text)) {
    gfx.setFreeFont(&font);
    return gfx.textWidth(text);
  }
  return mixedWidth(font, text);
}

void drawText(Seeed_GFX &gfx, const GFXfont &font, const char *text, int16_t x,
              int16_t y, uint8_t datum, uint16_t color) {
  if (hasThai(text)) {
    drawMixed(gfx, font, text, x, y, datum, color);
    return;
  }
  gfx.setFreeFont(&font);
  gfx.setTextDatum(datum);
  gfx.setTextColor(color);
  gfx.drawString(text, x, y);
}

int16_t drawDegreeText(Seeed_GFX &gfx, const GFXfont &font, const char *text,
                       const char *suffix, int16_t x, int16_t y, Align align,
                       uint16_t color) {
  gfx.setFreeFont(&font);
  const int16_t fh = gfx.fontHeight();
  const int16_t r = std::max<int16_t>(3, fh / 8);
  const int16_t gap = 2;
  const int16_t textW = textWidth(gfx, font, text);
  const int16_t suffixW = suffix ? textWidth(gfx, font, suffix) : 0;
  const int16_t total = textW + gap + 2 * r + gap + suffixW;

  int16_t sx = x;
  if (align == kCenter) {
    sx = x - total / 2;
  } else if (align == kRight) {
    sx = x - total;
  }

  drawText(gfx, font, text, sx, y, ML_DATUM, color);
  const int16_t cx = sx + textW + gap + r;
  const int16_t cy = y - fh / 4;
  gfx.drawCircle(cx, cy, r, color);
  gfx.drawCircle(cx, cy, r - 1, color);
  if (suffix) {
    drawText(gfx, font, suffix, cx + r + gap, y, ML_DATUM, color);
  }
  return total;
}

void thickLine(Seeed_GFX &gfx, float x0, float y0, float x1, float y1,
               float thickness, uint16_t color) {
  const float dx = x1 - x0;
  const float dy = y1 - y0;
  const float len = sqrtf(dx * dx + dy * dy);
  const float half = thickness / 2.0f;
  if (len >= 1.0f) {
    const float nx = -dy / len * half;
    const float ny = dx / len * half;
    gfx.fillTriangle(x0 + nx, y0 + ny, x0 - nx, y0 - ny, x1 + nx, y1 + ny,
                     color);
    gfx.fillTriangle(x1 + nx, y1 + ny, x0 - nx, y0 - ny, x1 - nx, y1 - ny,
                     color);
  }
  const int32_t capR = static_cast<int32_t>(half + 0.5f);
  gfx.fillCircle(x0, y0, capR, color);
  gfx.fillCircle(x1, y1, capR, color);
}

void formatNumber(char *out, size_t size, double value, uint8_t decimals) {
  char raw[32];
  snprintf(raw, sizeof(raw), "%.*f", decimals, fabs(value));
  const char *dot = strchr(raw, '.');
  const size_t intLen = dot ? static_cast<size_t>(dot - raw) : strlen(raw);

  size_t n = 0;
  if (value < 0 && n + 1 < size) {
    out[n++] = '-';
  }
  for (size_t i = 0; i < intLen && n + 1 < size; ++i) {
    out[n++] = raw[i];
    const size_t left = intLen - i - 1;
    if (left > 0 && left % 3 == 0 && n + 1 < size) {
      out[n++] = ',';
    }
  }
  for (const char *p = dot; p && *p && n + 1 < size; ++p) {
    out[n++] = *p;
  }
  out[n] = '\0';
}

} // namespace TextUtil
