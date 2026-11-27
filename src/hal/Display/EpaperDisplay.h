#pragma once

#include <Seeed_GFX.h>

class EpaperDisplay {
public:
  bool begin();
  GfxResult refresh() { return gfx_.refresh(); }
  Seeed_GFX &gfx() { return gfx_; }

private:
  Seeed_GFX gfx_;
};
