#include "ssd1306.h"
#include <avr/pgmspace.h>
#include "i2c.h"

#define SSD1306_MAX_COLUMN_ADDRESS (SSD1306_COLUMNS_N - 1)
#define SSD1306_MAX_PAGE_ADDRESS (SSD1306_PAGES_N - 1)


static const uint8_t SSD1306_INIT_SEQUENCE[] PROGMEM = {
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


void
ssd1306_init(ssd1306_t *device, uint8_t address)
{
  device->address = address;
  device->cursor_column = 0;
  device->cursor_page = 0;
  device->i2c_mode = SSD1306_I2C_MODE_NOT_SELECTED;
  i2c_transmit_progmem(device->address, SSD1306_INIT_SEQUENCE, sizeof(SSD1306_INIT_SEQUENCE));
}


void
ssd1306_start_update(ssd1306_t *device, ssd1306_update_callback_t callback, void *data)
{
  i2c_transmit_async(device->address, callback, data);
}


void
ssd1306_finish_update(ssd1306_t *device)
{
  device->i2c_mode = SSD1306_I2C_MODE_NOT_SELECTED;
  i2c_async_end_transmission();
}


static void
ssd1306_switch_i2c_mode(ssd1306_t *device, ssd1306_i2c_mode_t i2c_mode)
{
  if (device->i2c_mode != i2c_mode) {
    i2c_async_send_start();
    i2c_async_send_byte(i2c_mode);
    device->i2c_mode = i2c_mode;
  }
}


void
ssd1306_move_to(ssd1306_t *device, uint8_t column, uint8_t page)
{
  if (device->cursor_column != column || device->cursor_page != page) {
    ssd1306_switch_i2c_mode(device, SSD1306_I2C_MODE_COMMAND);

    i2c_async_send_byte(SSD1306_CMD_SET_COLUMN_ADDRESS);
    i2c_async_send_byte(column);
    i2c_async_send_byte(SSD1306_MAX_COLUMN_ADDRESS);
    i2c_async_send_byte(SSD1306_CMD_SET_PAGE_ADDRESS);
    i2c_async_send_byte(page);
    i2c_async_send_byte(SSD1306_MAX_PAGE_ADDRESS);

    device->cursor_column = column;
    device->cursor_page = page;
  }
}


void
ssd1306_write_gddram(ssd1306_t *device, uint8_t length, uint8_t *data)
{
  ssd1306_switch_i2c_mode(device, SSD1306_I2C_MODE_DATA);
  i2c_async_send_bytes(data, length);

  device->cursor_column += length;

  while (device->cursor_column >= SSD1306_COLUMNS_N) {
    device->cursor_column -= SSD1306_COLUMNS_N;
    ++device->cursor_page;
  }
}


void
ssd1306_put_segments(ssd1306_t *device, uint8_t column, uint8_t page, uint8_t width, uint8_t *segments)
{
  ssd1306_move_to(device, column, page);
  ssd1306_write_gddram(device, width, segments);
}
