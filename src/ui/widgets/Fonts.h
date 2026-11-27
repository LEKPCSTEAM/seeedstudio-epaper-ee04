#pragma once

#include <Seeed_GFX.h>

// Every accessor returns the face for Language::current().
namespace Fonts {

enum Id : uint8_t { kSans9, kSans12, kSans18, kBold9, kBold12, kBold18, kBold24, kCount };

const GFXfont &get(Id id);
// kCount when `font` is not one of ours.
Id idOf(const GFXfont &font);

inline const GFXfont &sans9() { return get(kSans9); }
inline const GFXfont &sans12() { return get(kSans12); }
inline const GFXfont &sans18() { return get(kSans18); }
inline const GFXfont &bold9() { return get(kBold9); }
inline const GFXfont &bold12() { return get(kBold12); }
inline const GFXfont &bold18() { return get(kBold18); }
inline const GFXfont &bold24() { return get(kBold24); }

} // namespace Fonts
