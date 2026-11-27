#pragma once

#include <Print.h>
#include <Seeed_GFX.h>

// Streams the frame buffer as hex rows (1 bit per pixel, 1 = black) for scripts/screenshot.py.
void dumpScreen(Seeed_GFX &gfx, Print &out);
