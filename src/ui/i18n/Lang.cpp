#include "ui/i18n/Lang.h"

#include <atomic>

namespace {
std::atomic<uint8_t> gLang{static_cast<uint8_t>(Lang::Th)};
} // namespace

namespace Language {

Lang current() { return static_cast<Lang>(gLang.load()); }

void set(Lang lang) { gLang.store(static_cast<uint8_t>(lang)); }

} // namespace Language
