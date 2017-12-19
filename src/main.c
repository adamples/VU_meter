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


region_t REGIONS[18] = { 0 };

update_extents_t UPDATE_EXTENTS = {
  .regions_n = 0,
  .regions = REGIONS
};


progmem_image_sprite_t background;
progmem_image_sprite_t peak_indicator;
needle_sprite_t needle_a;
needle_sprite_t needle_b;
ssd1306_t device_a;
ssd1306_t device_b;
display_t display_a;
display_t display_b;


void
status(char *str)
{
  lcd_clear();
  lcd_puts("Status:");
  lcd_goto(0, 1);
  lcd_puts(str);
}


int main(void)
{
  lcd_init();
  i2c_init();
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
  i2c_wait();

  uint8_t angle_a = 0;
  uint8_t angle_b = 0;

  while (1) {
    angle_a = 255 - adc_get(2) / 4;
    angle_b = 255 - adc_get(3) / 4;

    i2c_wait();

    update_extents_reset(&UPDATE_EXTENTS);
    needle_sprite_add_to_extents(&needle_a, &UPDATE_EXTENTS);

    needle_sprite_draw(&needle_a, angle_a);
    needle_sprite_draw(&needle_b, angle_b);
    peak_indicator.sprite.visible = (angle_a > 192);

    needle_sprite_add_to_extents(&needle_a, &UPDATE_EXTENTS);
    update_extents_optimize(&UPDATE_EXTENTS);

    //~ display_update_async(&display_a);
    //~ display_update_async(&display_b);
    display_update_partial_async(&display_a, &UPDATE_EXTENTS);
    //~ display_update_partial_async(&display_b, &UPDATE_EXTENTS);

    //~ uint16_t i = 0;
    //~ i2c_wait();
    //~ BENCHMARK(display_update, {
      //~ display_update_partial_async(&display_a, &UPDATE_EXTENTS);

      //~ while (!i2c_is_idle()) {
        //~ _delay_us(100);
        //~ ++i;
      //~ }
    //~ });
    //~ lcd_putc(' ');
    //~ lcd_put_int(i);
    //~ lcd_puts("  us");
    //~ _delay_ms(100);
  }
}
