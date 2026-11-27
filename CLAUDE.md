# EE04 ePaper firmware

Live desk dashboard for the Seeed **XIAO ePaper Display Board EE04** (XIAO ESP32-S3 Plus,
16MB flash, 8MB OPI PSRAM) driving a **7.5" monochrome ePaper, 800x480, UC8179**
through Seeed_GFX2. Wiki: https://wiki.seeedstudio.com/epaper_ee04/

Screen: dot-matrix clock + date, current weather + US AQI, 4-day forecast,
month calendar, battery, markets, 24 h temperature graph.

## Languages (EN / TH)

- KEY2 (or serial `L` in debug) toggles; the choice is saved in NVS `ui/lang`, default Thai.
  Thai mode uses Buddhist-era years and the Prompt typeface for all text.
- All UI text lives in `src/ui/i18n/Strings.cpp`. Widgets never hold literals.
- Thai is drawn from runs pre-shaped offline (Pillow + raqm), so stacked marks are
  correct without a shaping engine on the MCU. After adding or changing any Thai
  literal, regenerate, then build:

```bash
python3 scripts/gen_thai_assets.py   # system python3: needs Pillow with raqm
```

- A run is U+0E00..U+0E7F plus `.` `/`; never join two Thai strings without a space.
  A run missing at runtime draws a hollow box and logs `missing Thai run`.
- UTF-8 Thai is 3 bytes per character: size `char` buffers for it.
- Fonts: `assets/fonts/Prompt-*.ttf` (OFL) -> `src/ui/fonts/Prompt_*.h` (ASCII) and
  `src/ui/text/ThaiRunsData.cpp` (~175 KB of run bitmaps).

## Data sources (no API keys)

| Data | Source | Notes |
|---|---|---|
| Forecast | `api.open-meteo.com/v1/forecast` | `timeformat=unixtime`, Bangkok lat/lon in `src/config/AppConfig.h` |
| Air quality | `air-quality-api.open-meteo.com/v1/air-quality` | `us_aqi`, `pm2_5` |
| Quotes | `query1.finance.yahoo.com/v8/finance/chart/<sym>` | needs `User-Agent`; symbols in `AppConfig::kStocks` |
| Time | NTP, POSIX TZ `ICT-7` | resynced on every fetch |

TLS is verified against roots in `src/net/RootCerts.h` (ISRG X1/X2 for Open-Meteo,
DigiCert G2/G3 for Yahoo). If a host changes CA, fetches fail with a TLS error
in the log: re-check with `openssl s_client` and update that file.

WiFi credentials live in `src/config/Env.h` (gitignored); template `Env.h.example`.

## Refresh policy

- Data fetch every 15 min (60 s retry on failure or unsynced time). WiFi is off between fetches.
- New data: render whole page, partial refresh (~1.5 s, no flash).
- Each minute: redraw `DashboardPage::clockRect()` only, partial refresh (~1.2 s).
- Each hour, first draw, KEY1, or a language switch: full refresh (~2.6 s) to clear ghosting.
- KEY0: fetch now. KEY2: toggle language.

## Debug tools (ee04_debug only)

Serial commands: `S` dumps the frame buffer, `F` forces a fetch, `L` toggles language.

```bash
~/.platformio/penv/bin/python scripts/screenshot.py /dev/cu.usbmodem13201 shot.png
```

Arduino core 3.x gotchas hit here: `NetworkClient::setTimeout` is in **ms**;
parsing JSON straight off the TLS stream truncated Yahoo bodies, so `HttpsJson`
reads the whole body with `getString()` first; the RTC keeps stale time across
resets, so time is trusted only after `sntp_get_sync_status()` completes.

## Build

Use the PlatformIO Core inside `~/.platformio/penv` (6.2.0). The Homebrew
`pio` on PATH is 6.1.19 and refuses the pioarduino platform.

```bash
PIO=~/.platformio/penv/bin/pio
$PIO run -e ee04_debug
$PIO run -e ee04_release
$PIO run -e ee04_release -t upload
$PIO device monitor
$PIO run -e ee04_debug -t compiledb   # refresh compile_commands.json for clangd
```

Hardware: the 24-pin / 50-pin FPC jumper on the EE04 must match the panel.

## Dependencies

| Item | Version | Why |
|---|---|---|
| pioarduino platform-espressif32 | `55.03.312-1` (Arduino core 3.3.12) | Seeed_GFX2 needs core 3.x; `espressif32@7.0.1` (core 2.0.17) fails to compile it |
| board | `seeed_xiao_esp32_s3_plus` + `default_16MB.csv` | matches the S3 Plus module |
| Seeed_GFX2 | git `3739fd5` (no tags upstream) | display stack |
| ArduinoJson | 7.4.3 | API parsing with filters |

## Layout

```
src/
├── main.cpp                    setup()/loop() delegate to App
├── version.h                   FIRMWARE_VERSION
├── config/                     AppConfig.h (location, symbols, intervals), Env.h (WiFi, gitignored)
├── app/App.*                   singleton, owns modules, tasks, shared DashboardData
├── app/model/DashboardData.h   data passed from services to the UI
├── app/services/               WeatherService (Open-Meteo), StocksService (Yahoo)
├── net/                        WifiLink (WiFi + SNTP), HttpsJson (verified GET -> JSON), RootCerts
├── hal/Display/                EpaperDisplay (EE04 board config from pin macros), ScreenDump
├── hal/Buttons/Buttons.*       KEY0..KEY2, debounced, active low
├── hal/Battery/                Battery (calibrated VBAT mV), BatteryLevel (filter + LiPo curve)
└── ui/
    ├── pages/DashboardPage.*   layout; all zone coordinates are constexpr at the top
    ├── widgets/                DotClock, WeatherIcon, MonthCalendar, StockList, TempGraph,
    │                           BatteryGauge, TextUtil (mixed Thai/ASCII text), Fonts
    ├── i18n/                   Lang (current language), Strings (EN/TH text)
    ├── text/                   ThaiRuns lookup + generated ThaiRunsData.cpp
    └── fonts/                  generated Prompt GFX fonts
```

Font headers are included only in `ui/widgets/Fonts.cpp` so font data is not duplicated.
`Fonts::get()` returns FreeSans in English and Prompt in Thai; layouts measure text
with `TextUtil::textWidth` and fall back to smaller fonts when a line would overflow.

- Includes are rooted at `src/` (`#include "hal/Battery/Battery.h"`).
- Seeed_GFX2 also ships a `ui/` include tree; keep file names under our
  `src/ui/` distinct from the library's to avoid include shadowing.
- The display uses `begin<Ee04Board, Config_Seeed_ePaper_7inch5_BW_UC8179>()`
  rather than the `Seeed_ePaper_7INCH5` product enum, so pins stay in
  `platformio.ini`. SPI timing (10 MHz, HSPI) is inherited from the library's
  `XIAO_ePaper_CommonConfig`.

## Tasks

| Task | Stack | Priority | Role |
|---|---|---|---|
| `loop()` | Arduino default | 1 | poll keys every 10 ms, serial commands (debug), notify tasks |
| `net` | 16384 | 1 | WiFi on, SNTP, fetch weather/AQI/quotes, WiFi off, notify display |
| `display` | 8192 | 1 | battery sample, page/clock render and every panel refresh, wakes each minute |

Only the `display` task touches the panel; it takes event bits via `xTaskNotify`
(`kEvtFull`, `kEvtData`, `kEvtDump`). `data_` is shared under `dataMutex_`.

## Pins

All pins are `-D` defines in `[pins]` of `platformio.ini`; no GPIO literal in source.

| Macro | GPIO | Function |
|---|---|---|
| `EPD_SCK` | 7 | ePaper SPI clock (XIAO D8) |
| `EPD_MOSI` | 9 | ePaper SPI data (XIAO D10) |
| `EPD_CS` | 44 | ePaper chip select |
| `EPD_DC` | 10 | ePaper data/command |
| `EPD_RST` | 38 | ePaper reset |
| `EPD_BUSY` | 4 | ePaper busy input |
| `EPD_PWR_EN` | 43 | ePaper power enable, active high |
| `KEY0_PIN` | 2 | user key, active low |
| `KEY1_PIN` | 3 | user key, active low |
| `KEY2_PIN` | 5 | user key, active low |
| `BAT_ADC_PIN` | 1 | battery divider ADC |
| `BAT_ADC_EN` | 6 | high connects the battery divider |

## Battery

- VBAT = `analogReadMilliVolts` (eFuse-calibrated) x 2: schematic v1.2 divider R28/R29
  is 10K/10K 1% behind a TPS22916 switch on `BAT_ADC_EN`. 32 samples, middle-half mean.
  The Seeed wiki's `raw / 4096 * 7.16` read ~70 mV high on this board; it is not used.
- Percent = resting-voltage (OCV) curve for a single LiPo cell, linear between points,
  in `hal/Battery/BatteryLevel.cpp`. Samples are taken each minute by the `display`
  task, only while WiFi is off (`radioOn_`), and smoothed (EMA, ~4 min).
- Per-board trim: measure VBAT with a multimeter, set
  `AppConfig::kBatteryCalScale = multimeter mV / logged mV`.
- Limits: no charge-status line reaches the MCU (SY6974B STAT drives only the LED),
  so while charging the percent reads high. A generic curve is good to roughly
  +-5 % at rest; cell age and temperature shift it.
