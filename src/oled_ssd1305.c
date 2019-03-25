#include "oled.h"
#include <avr/pgmspace.h>
#include "i2c.h"

#define OLED_MAX_COLUMN_ADDRESS (OLED_COLUMNS_N - 1)
#define OLED_MAX_PAGE_ADDRESS (OLED_PAGES_N - 1)


static const uint8_t OLED_INIT_SEQUENCE[] PROGMEM = {
  OLED_CTRL_COMMAND | OLED_CTRL_N_BYTES,
  0xAE, //  command(SSD1305_DISPLAYOFF);
  0x04, // command(SSD1305_SETLOWCOLUMN | 0x4);
  0x14, // command(SSD1305_SETHIGHCOLUMN | 0x4);
  0x40, // command(SSD1305_SETSTARTLINE | 0x0);
  0x2E, // command(0x2E); // stop scrolling

  0x81, // command(SSD1305_SETCONTRAST);
  0x32, // command(0x32);

  0x82, // command(SSD1305_SETBRIGHTNESS);
  0x80, // command(0x80);

  0xA1, // command(SSD1305_SEGREMAP | 0x01);
  0xA6, // command(SSD1305_NORMALDISPLAY);

  0xA8, // command(SSD1305_SETMULTIPLEX);
  0x3F, // command(0x3F); // 1/64

  0xAD, // command(SSD1305_MASTERCONFIG);
  0x8E, //  command(0x8e); /* external vcc supply */

  0xC8, // command(SSD1305_COMSCANDEC);

  0xD3, // command(SSD1305_SETDISPLAYOFFSET);
  0x40, // command(0x40);

  0xD5, // command(SSD1305_SETDISPLAYCLOCKDIV);
  0xF0, // command(0xf0);

  0xD8, // command(SSD1305_SETAREACOLOR);
  0x05, // command(0x05);

  0xD9, // command(SSD1305_SETPRECHARGE);
  0xF1, // command(0xF1);

  0xDA, // command(SSD1305_SETCOMPINS);
  0x12, // command(0x12);

  0x91, // command(SSD1305_SETLUT);
  0x3F, // command(0x3F);
  0x3F, // command(0x3F);
  0x3F, // command(0x3F);
  0x3F, // command(0x3F);

  0xA5, // command(SSD1305_DISPLAYON);
};


void
oled_draw_cmd_init(oled_draw_cmd_t *draw)
{
  draw->ctrl1 = OLED_CTRL_COMMAND | OLED_CTRL_SINGLE_BYTE;
  draw->ctrl2 = OLED_CTRL_COMMAND | OLED_CTRL_SINGLE_BYTE;
  draw->ctrl3 = OLED_CTRL_COMMAND | OLED_CTRL_SINGLE_BYTE;
  draw->ctrl4 = OLED_CTRL_DATA | OLED_CTRL_N_BYTES;
}


void
oled_draw_cmd_set_address(oled_draw_cmd_t *draw, uint8_t address)
{
  draw->address = address;
}


void
oled_draw_cmd_set_dimensions(oled_draw_cmd_t *draw, uint8_t column, uint8_t page, uint8_t width)
{
  draw->column = column + OLED_COLUMN_OFFSET;
  draw->page = page;
  draw->width = width;
  draw->cmd_high_column = OLED_CMD_SET_HIGH_COLUMN(draw->column >> 4);
  draw->cmd_low_column = OLED_CMD_SET_LOW_COLUMN(draw->column & 0x0f);
  draw->cmd_page = OLED_CMD_SET_PAGE_START_ADDRESS(draw->page);
}


oled_segment_t *
oled_draw_cmd_get_segments(oled_draw_cmd_t *draw)
{
  return draw->gddram_data;
}


bool
oled_init(oled_t *device, uint8_t address)
{
  device->address = address;
  device->column = 255;
  device->page = 255;
  uint8_t status = i2c_transmit_progmem(address, sizeof(OLED_INIT_SEQUENCE), OLED_INIT_SEQUENCE);
  return (status == TW_OK);
}


void
oled_set_display_on(oled_t *device, bool enabled)
{
  uint8_t OLED_ON_SEQUENCE[] = {
    device->address,
    OLED_CTRL_COMMAND | OLED_CTRL_N_BYTES,
    OLED_CMD_SET_DISPLAY_ON(enabled)
  };

  i2c_transmit(sizeof(OLED_ON_SEQUENCE), OLED_ON_SEQUENCE);
  i2c_wait();
}


void
oled_draw_cmd_start(oled_draw_cmd_t *draw, oled_t *device)
{
  oled_draw_cmd_set_address(draw, device->address);

  if ((device->column == draw->column) && (device->page == draw->page)) {
    draw->cmd_page = draw->address;
    i2c_transmit(draw->width + 2, (uint8_t *) &(draw->cmd_page));
  }
  else {
    i2c_transmit(draw->width + 8, (uint8_t *) &(draw->address));
  }

  device->page = draw->page;
  device->column = draw->column + draw->width;
}


bool
oled_draw_cmd_finish()
{
  return (i2c_wait() == TW_OK);
}
