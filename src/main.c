#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "assert.h"
#include "i2c.h"
#include "lcd.h"
#include "images.h"
#include "ssd1306.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "benchmark.h"
#include "adc.h"


region_t REGIONS[8] = {
  { .page = 0, .start_column = 52, .end_column = 75 },
  { .page = 1, .start_column = 52, .end_column = 75 },
  { .page = 2, .start_column = 52, .end_column = 75 },
  { .page = 3, .start_column = 52, .end_column = 75 },
  { .page = 4, .start_column = 52, .end_column = 75 },
  { .page = 5, .start_column = 52, .end_column = 75 },
  { .page = 6, .start_column = 52, .end_column = 75 },
  { .page = 7, .start_column = 52, .end_column = 75 }
};

update_extents_t UPDATE_EXTENTS = {
  .regions_n = 8,
  .regions = REGIONS
};

int main(void)
{
  progmem_image_sprite_t background;
  progmem_image_sprite_t peak_indicator;
  needle_sprite_t needle_a;
  needle_sprite_t needle_b;
  ssd1306_t device_a;
  ssd1306_t device_b;
  display_t display_a;
  display_t display_b;

  lcd_init();
  i2c_init();
  lcd_puts("Started");
  sei();

  progmem_image_sprite_init(&background, BACKGROUND, 0, 0);
  progmem_image_sprite_init(&peak_indicator, PEAK_INDICATOR, 107, 7);
  needle_sprite_init(&needle_a);
  needle_sprite_init(&needle_b);

  ssd1306_init(&device_a, DISPLAY_A_ADDRESS);
  ssd1306_init(&device_b, DISPLAY_B_ADDRESS);
  display_init(&display_a, &device_a);
  display_init(&display_b, &device_b);

  display_add_sprite(&display_a, &background.sprite);
  display_add_sprite(&display_a, &peak_indicator.sprite);
  display_add_sprite(&display_a, &needle_a.sprite);

  display_add_sprite(&display_b, &background.sprite);
  display_add_sprite(&display_b, &peak_indicator.sprite);
  display_add_sprite(&display_b, &needle_b.sprite);

  needle_sprite_draw(&needle_a, 64);

  display_update_async(&display_a);
  display_update_async(&display_b);
  _delay_ms(100);

  //~ uint16_t i = 0;
  //~ i2c_wait();
  //~ BENCHMARK(display_update, {
    //~ display_update_partial_async(&display_a, &UPDATE_EXTENTS);

    //~ while (!i2c_is_idle()) {
      //~ _delay_us(100);
      //~ ++i;
    //~ }
  //~ });

  uint8_t angle_a = 0;
  uint8_t angle_b = 0;

  while (1) {
    angle_a = 255 - adc_get(2) / 4;
    angle_b = 255 - adc_get(3) / 4;

    i2c_wait();

    needle_sprite_draw(&needle_a, angle_a);
    needle_sprite_draw(&needle_b, angle_b);
    peak_indicator.sprite.visible = (angle_a > 192);

    display_update_partial_async(&display_a, &UPDATE_EXTENTS);
    display_update_partial_async(&display_b, &UPDATE_EXTENTS);
  }
}
