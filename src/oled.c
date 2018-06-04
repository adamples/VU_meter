#include "config.h"

#if OLED_DRIVER == OLED_DRIVER_SSD1306
#include "oled_ssd1306.c"
#else
#include "oled_sh1106.c"
#endif
