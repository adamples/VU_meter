#ifndef OLED_H_STUB
#define OLED_H_STUB

#include "config.h"

#if OLED_DRIVER == OLED_DRIVER_SSD1305
#include "oled_ssd1305.h"
#elif OLED_DRIVER == OLED_DRIVER_SSD1306
#include "oled_ssd1306.h"
#elif OLED_DRIVER == OLED_DRIVER_SSD1309
#include "oled_ssd1309.h"
#else
#include "oled_sh1106.h"
#endif

void oled_reset();

#endif
