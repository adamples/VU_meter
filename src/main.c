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
#include "drawing.h"
#include "benchmark.h"



#define SSD1306_DEFAULT_ADDRESS 0x7a
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_NOP 0xE3


static const uint8_t OLED_INIT_SEQUENCE[] = {
    0x00,
    SSD1306_DISPLAYOFF,
    SSD1306_SETMULTIPLEX, 0x3F,
    SSD1306_SETDISPLAYOFFSET, 0x00,
    SSD1306_SETSTARTLINE | 0x00,
    SSD1306_MEMORYMODE, 0x00,
    SSD1306_SEGREMAP | 0x1, SSD1306_COMSCANDEC,
    /* rotated: SSD1306_SEGREMAP | 0x0, SSD1306_COMSCANINC */
    SSD1306_SETCOMPINS, 0x12,
    SSD1306_SETPRECHARGE, 0x11,
    SSD1306_SETVCOMDETECT, 0x40,
    SSD1306_DISPLAYALLON_RESUME,
    SSD1306_NORMALDISPLAY,
    SSD1306_SETDISPLAYCLOCKDIV, 0xf0,
    SSD1306_SETCONTRAST, 0xcf,
    SSD1306_CHARGEPUMP, 0x14,
    SSD1306_DISPLAYON
};

typedef struct write_const_t_ {
  uint16_t length;
  uint16_t counter;
  uint8_t *data;
} write_const_t;


void
i2c_write_const_cb(void *data)
{
  write_const_t *wc = (write_const_t *) data;

  if (wc->counter < wc->length) {
    for (uint8_t i = 0; i < 16 && wc->counter < wc->length; ++i) {
      i2c_async_send_data(wc->data[wc->counter]);
      ++wc->counter;
    }
  }
  else {
    wc->counter = 0;
    i2c_async_end_transmission();
  }
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

  ssd1306_init(&device_a, DISPLAY_A_ADDRESS);
  ssd1306_init(&device_b, DISPLAY_B_ADDRESS);
  display_init(&display_a, &device_a);
  display_init(&display_b, &device_b);

  BENCHMARK(oled_init, {
    i2c_transmit_async(0x78, i2c_write_const_cb, &oled_init_sequence);
    while (!i2c_is_idle()) _delay_us(100);
  });

  i2c_transmit_async(0x7a, i2c_write_const_cb, &oled_init_sequence);

  display_add_sprite(&display_a, &background.sprite);
  display_add_sprite(&display_a, &peak_indicator.sprite);
  display_add_sprite(&display_a, &needle_a.sprite);

  display_add_sprite(&display_b, &background.sprite);
  display_add_sprite(&display_b, &peak_indicator.sprite);
  display_add_sprite(&display_b, &needle_a.sprite);

  float angle = -0.73;
  float v = 0.05;
  float a = -0.001;

  while (1) {
    v += a;
    angle += v;
    if (angle < -0.73) {
      angle = -0.73 * 2 - angle;
      v = -v * 0.5;
    }

    float dx = sin(angle);
    float dy = cos(angle);
    float x = 64 + dx * 96;
    float y = 96 - dy * 96;

    needle_sprite_draw(&needle_a, x, y, 64, 96);
    peak_indicator.sprite.visible = (x > 90);
    display_update_async(&display_a);
    display_update_async(&display_b);

    _delay_ms(50);
  }
}
