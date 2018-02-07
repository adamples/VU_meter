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
  display_add_sprite(&display_b, &needle_a.sprite);

  peak_indicator.sprite.visible = false;
  needle_sprite_draw(&needle_a, 0);
  needle_sprite_draw(&needle_b, 0);
  display_update_async(&display_a);
  display_update_async(&display_b);

  i2c_wait();

  while (1) {
    uint16_t adc_a = adc_get(2);
    int16_t angle_a = (int32_t) (498 - adc_a) * 128 / 164;
    int16_t angle_b = 255 - adc_get(3) / 2;

    if (angle_a < 0) angle_a = -angle_a;

    lcd_goto(0, 0);
    lcd_put_int(2426 - (int32_t) adc_a * 5000 / 1024);
    lcd_puts("mV ");
    lcd_goto(8, 0);
    lcd_put_int(498 - adc_a);
    lcd_puts("    ");

    i2c_wait();

    update_extents_reset(&UPDATE_EXTENTS);
    needle_sprite_add_to_extents(&needle_a, &UPDATE_EXTENTS);

    needle_sprite_draw(&needle_a, angle_a);
    needle_sprite_draw(&needle_b, angle_b);

    needle_sprite_add_to_extents(&needle_a, &UPDATE_EXTENTS);
    update_extents_optimize(&UPDATE_EXTENTS);

    //~ display_update_async(&display_a);
    //~ display_update_async(&display_b);
    display_update_partial_async(&display_a, &UPDATE_EXTENTS);
    display_update_partial_async(&display_b, &UPDATE_EXTENTS);

    uint16_t i = 0;
    i2c_wait();
    BENCHMARK(display_update, {
      display_update_async(&display_a);
      // display_update_partial_async(&display_a, &UPDATE_EXTENTS);

      while (!i2c_is_idle()) {
        _delay_us(100);
        ++i;
      }
    });
    lcd_putc(' ');
    lcd_put_int(i);
    lcd_puts("00us");
    _delay_ms(500);
  }
}
