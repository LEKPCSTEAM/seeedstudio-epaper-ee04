#include "ui/widgets/Fonts.h"

#include <font/GFXFF/FreeSans12pt7b.h>
#include <font/GFXFF/FreeSans18pt7b.h>
#include <font/GFXFF/FreeSans9pt7b.h>
#include <font/GFXFF/FreeSansBold12pt7b.h>
#include <font/GFXFF/FreeSansBold18pt7b.h>
#include <font/GFXFF/FreeSansBold24pt7b.h>
#include <font/GFXFF/FreeSansBold9pt7b.h>

#include "ui/fonts/Prompt_Regular_17.h"
#include "ui/fonts/Prompt_Regular_25.h"
#include "ui/fonts/Prompt_Regular_36.h"
#include "ui/fonts/Prompt_SemiBold_17.h"
#include "ui/fonts/Prompt_SemiBold_25.h"
#include "ui/fonts/Prompt_SemiBold_36.h"
#include "ui/fonts/Prompt_SemiBold_48.h"
#include "ui/i18n/Lang.h"

namespace {

const GFXfont *const kEnglish[Fonts::kCount] = {
    &FreeSans9pt7b,     &FreeSans12pt7b,     &FreeSans18pt7b,    &FreeSansBold9pt7b,
    &FreeSansBold12pt7b, &FreeSansBold18pt7b, &FreeSansBold24pt7b};

const GFXfont *const kThai[Fonts::kCount] = {
    &Prompt_Regular_17,  &Prompt_Regular_25,  &Prompt_Regular_36, &Prompt_SemiBold_17,
    &Prompt_SemiBold_25, &Prompt_SemiBold_36, &Prompt_SemiBold_48};

} // namespace

namespace Fonts {

const GFXfont &get(Id id) {
  const GFXfont *const *set = Language::current() == Lang::Th ? kThai : kEnglish;
  return *set[id < kCount ? id : kSans9];
}

Id idOf(const GFXfont &font) {
  for (uint8_t i = 0; i < kCount; ++i) {
    if (kEnglish[i] == &font || kThai[i] == &font) {
      return static_cast<Id>(i);
    }
  }
  return kCount;
}

} // namespace Fonts
