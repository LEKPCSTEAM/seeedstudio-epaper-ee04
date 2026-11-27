#include "app/services/StocksService.h"

#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "net/HttpsJson.h"

namespace {

void appendEncoded(char *dst, size_t cap, const char *src) {
  size_t n = strlen(dst);
  for (; *src != '\0' && n + 4 < cap; src++) {
    const char c = *src;
    if (isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_') {
      dst[n++] = c;
    } else {
      n += snprintf(dst + n, cap - n, "%%%02X", static_cast<unsigned char>(c));
    }
  }
  dst[n] = '\0';
}

bool fetchQuote(const AppConfig::StockSymbol &s, Quote &q) {
  char url[160] = "https://query1.finance.yahoo.com/v8/finance/chart/";
  appendEncoded(url, sizeof(url), s.symbol);
  strlcat(url, "?range=1d&interval=1d", sizeof(url));

  JsonDocument filter;
  filter["chart"]["result"][0]["meta"]["regularMarketPrice"] = true;
  filter["chart"]["result"][0]["meta"]["chartPreviousClose"] = true;

  JsonDocument doc;
  if (!HttpsJson::get(url, filter, doc, "Mozilla/5.0")) {
    return false;
  }
  JsonObjectConst meta = doc["chart"]["result"][0]["meta"];
  if (meta["regularMarketPrice"].isNull() || meta["chartPreviousClose"].isNull()) {
    log_w("quote json missing fields: %s", s.symbol);
    return false;
  }
  strlcpy(q.symbol, s.symbol, sizeof(q.symbol));
  strlcpy(q.label, s.label, sizeof(q.label));
  q.price = meta["regularMarketPrice"];
  q.previousClose = meta["chartPreviousClose"];
  q.valid = true;
  return true;
}

} // namespace

bool StocksService::fetch(StocksData &out) {
  constexpr size_t kCount = sizeof(AppConfig::kStocks) / sizeof(AppConfig::kStocks[0]);
  static_assert(kCount <= kQuoteCount, "more symbols than quote slots");

  bool any = false;
  for (size_t i = 0; i < kCount; i++) {
    if (fetchQuote(AppConfig::kStocks[i], out.quotes[i])) {
      any = true;
    } else {
      log_w("quote failed: %s", AppConfig::kStocks[i].symbol);
    }
  }
  if (any) {
    out.updatedAt = time(nullptr);
  }
  return any;
}
