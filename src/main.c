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

  adc_init();
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

  uint16_t i = 0;

  i2c_wait();

  //~ BENCHMARK(display_update, {
    //~ display_update_async(&display_a);

    //~ while (!i2c_is_idle()) {
      //~ _delay_us(100);
      //~ ++i;
    //~ }
  //~ });

  lcd_goto(8, 1);
  lcd_put_int(i);
  lcd_puts("00us");

  //~ while (1);

  uint8_t angle = 0;
  uint8_t angle2 = 0;

  while (1) {
    angle = adc_get() / 4;
    angle2 = (uint16_t) (angle * 1 + angle2 * 7) / 8;

    i2c_wait();

    needle_sprite_draw(&needle_a, angle);
    needle_sprite_draw(&needle_b, angle2);
    peak_indicator.sprite.visible = (angle > 192);

    display_update_async(&display_a);
    display_update_async(&display_b);

    //~ lcd_clear();
    //~ lcd_put_int(angle * 1000);
    //~ lcd_puts("rad");
    //~ _delay_ms(500);
  }
}
