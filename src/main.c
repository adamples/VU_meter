#include "i2c.h"
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "assert.h"
#include "lcd.h"
#include "images.h"
#include "ssd1306.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "benchmark.h"
#include "adc.h"


static const uint8_t OLED_INIT_SEQUENCE[] = {
    0x00,
    SSD1306_CMD_SET_DISPLAY_ON(false),
    SSD1306_CMD_SET_MULTIPLEX_RATIO, 0x3F,
    SSD1306_CMD_SET_DISPLAY_OFFSET, 0x00,
    SSD1306_CMD_SET_DISPLAY_START_LINE(0x00),
    SSD1306_CMD_SET_MEMORY_ADDRESSING_MODE, 0x00,
    SSD1306_CMD_SET_SEGMENT_REMAP(true), SSD1306_CMD_SET_COM_SCAN_DIRECTION_DEC,
    /* rotated: SSD1306_SEGREMAP | 0x0, SSD1306_COMSCANINC */
    SSD1306_CMD_SET_COM_PINS_HW_CONF, 0x12,
    SSD1306_CMD_SET_PRECHARGE_PERIOD, 0x11,
    SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL, 0x40,
    SSD1306_CMD_ENTIRE_DISPLAY_ON(false),
    SSD1306_CMD_SET_INVERSED(false),
    SSD1306_CMD_SET_CLOCK_DIVIDE_FREQUENCY, 0xf0,
    SSD1306_CMD_SET_CONTRAST, 0xcf,
    SSD1306_CMD_CHARGE_PUMP_SETTING, 0x14,
    SSD1306_CMD_SET_DISPLAY_ON(true),
    SSD1306_CMD_SET_COLUMN_ADDRESS,
    0x00, 0x7f,
    SSD1306_CMD_SET_PAGE_ADDRESS,
    0x00, 0x07
};


typedef struct write_const_t_ {
  uint16_t length;
  uint16_t counter;
  uint8_t *data;
} write_const_t;


bool
i2c_write_const_cb(void *data)
{
  write_const_t *wc = (write_const_t *) data;

  if (wc->counter == 0) {
    i2c_async_send_start();
  }

  if (wc->counter < wc->length) {
    for (uint8_t i = 0; i < 16 && wc->counter < wc->length; ++i) {
      i2c_async_send_byte(wc->data[wc->counter]);
      ++wc->counter;
    }

    return true;
  }

  wc->counter = 0;
  i2c_async_end_transmission();
  return false;
}


int main(void)
{
  write_const_t oled_init_sequence = {
    .length = sizeof(OLED_INIT_SEQUENCE),
    .counter = 0,
    .data = (uint8_t *) OLED_INIT_SEQUENCE
  };

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

  i2c_transmit_async(DISPLAY_A_ADDRESS, i2c_write_const_cb, &oled_init_sequence);
  i2c_transmit_async(DISPLAY_B_ADDRESS, i2c_write_const_cb, &oled_init_sequence);
  display_update_async(&display_a);
  display_update_async(&display_b);
  _delay_ms(100);

  uint16_t i = 0;

  while (!i2c_is_idle());

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

    while (!i2c_is_idle());

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
