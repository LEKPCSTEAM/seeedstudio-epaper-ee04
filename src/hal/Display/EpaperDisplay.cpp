#include "hal/Display/EpaperDisplay.h"

#include "board/ConfiguredSpiBoard.h"
#include "board/configs/XIAO_ePaper_Board_Configs.h"
#include "driver/epaper/Driver_UC8179.h"
#include "panel/Panel_EPaper.h"

namespace {

// Inherits the HSPI timing Seeed verified for EE boards; pins come from platformio.ini.
struct Ee04PinConfig : XIAO_ePaper_CommonConfig {
  static const char *name() { return "XIAO ePaper EE04"; }
  static BoardPinConfig pins() {
    return BoardPinConfig(EPD_CS, EPD_DC, EPD_RST, -1, EPD_MOSI, -1, EPD_SCK,
                          -1, -1, EPD_BUSY, EPD_PWR_EN);
  }
};

using Ee04Board = ConfiguredSpiBoard<Ee04PinConfig>;

} // namespace

bool EpaperDisplay::begin() {
  return gfx_.begin<Ee04Board, Config_Seeed_ePaper_7inch5_BW_UC8179>();
}
