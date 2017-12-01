#include "ssd1306.h"
#include "i2c.h"

#define SSD1306_MAX_COLUMN_ADDRESS (SSD1306_COLUMNS_N - 1)
#define SSD1306_MAX_PAGE_ADDRESS (SSD1306_PAGES_N - 1)


void
ssd1306_init(ssd1306_t *device, uint8_t address)
{
  device->address = address;
  device->cursor_column = 0;
  device->cursor_page = 0;
  device->i2c_mode = SSD1306_I2C_MODE_NOT_SELECTED;
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
