#include "i2c.h"
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "assert.h"
#include "images.h"
#include "ssd1306.h"


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
    i2c_async_send_data(wc->data[wc->counter]);
    ++wc->counter;
  }
  else {
    wc->counter = 0;
    i2c_async_end_transmission();
  }
}


typedef struct put_image_t_ {
  uint16_t x, y;
  uint16_t length;
  uint16_t counter;
  const uint8_t *data;
} put_image_t;


#define min(a, b) (((a) < (b)) ? (a) : (b))

void
ssd1306_put_image_P_cb(void *data)
{
  put_image_t *pi = (put_image_t *) data;

  if (pi->counter == 0) {
    uint8_t width = pgm_read_byte(pi->data);
    uint8_t height = pgm_read_byte(pi->data + 1);

    i2c_async_send_data(SSD1306_I2C_MODE_COMMAND);
    i2c_async_send_data(SSD1306_CMD_SET_COLUMN_ADDRESS);
    i2c_async_send_data(pi->x);
    i2c_async_send_data(min(pi->x + width - 1, 127));
    i2c_async_send_data(SSD1306_CMD_SET_PAGE_ADDRESS);
    i2c_async_send_data(pi->y / 8);
    i2c_async_send_data(min(pi->y / 8 + height / 8 - 1, 7));
    i2c_async_send_repeated_start();
    i2c_async_send_data(SSD1306_I2C_MODE_DATA);
  }

  if (pi->counter < pi->length) {
    uint8_t segment = pgm_read_byte(pi->data + 2 + pi->counter);
    i2c_async_send_data(segment);
    ++pi->counter;
  }
  else {
    i2c_async_end_transmission();
    free(pi);
  }
}


void
ssd1306_put_image_P(uint8_t address, const uint8_t *data, uint8_t x, uint8_t y)
{
  put_image_t *control_block = (put_image_t *) malloc(sizeof(put_image_t));
  uint8_t width = pgm_read_byte(data);
  uint8_t height = pgm_read_byte(data + 1);

  control_block->x = x;
  control_block->y = y;
  control_block->length = (uint16_t) width * height / 8;
  control_block->counter = 0;
  control_block->data = data;

  i2c_transmit_async(address, ssd1306_put_image_P_cb, control_block);
}


int main(void)
{
  write_const_t oled_init_sequence = {
    .length = sizeof(OLED_INIT_SEQUENCE),
    .counter = 0,
    .data = (uint8_t *) OLED_INIT_SEQUENCE
  };

  i2c_init();
  sei();

  i2c_transmit_async(0x78, i2c_write_const_cb, &oled_init_sequence);
  i2c_transmit_async(0x7a, i2c_write_const_cb, &oled_init_sequence);
  ssd1306_put_image_P(0x7a, BACKGROUND, 0, 0);

  _delay_ms(100);

  while (1) {
    ssd1306_put_image_P(0x78, BACKGROUND, 0, 0);
    ssd1306_put_image_P(0x7a, BACKGROUND, 0, 0);
    ssd1306_put_image_P(0x78, PEAK_INDICATOR, 107, 56);
    _delay_ms(500);
    ssd1306_put_image_P(0x78, BACKGROUND, 0, 0);
    ssd1306_put_image_P(0x7a, BACKGROUND, 0, 0);
    ssd1306_put_image_P(0x7a, PEAK_INDICATOR, 107, 56);
    _delay_ms(500);
  };
}
