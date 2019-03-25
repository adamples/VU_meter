#include "config.h"

#if OLED_DRIVER == OLED_DRIVER_SSD1305
#include "oled_ssd1305.c"
#elif OLED_DRIVER == OLED_DRIVER_SSD1306
#include "oled_ssd1306.c"
#elif OLED_DRIVER == OLED_DRIVER_SSD1309
#include "oled_ssd1309.c"
#else
#include "oled_sh1106.c"
#endif
