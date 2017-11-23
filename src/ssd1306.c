#include "ssd1306.h"
#include "i2c.h"

#define SSD1306_MAX_COLUMN_ADDRESS (127)
#define SSD1306_MAX_PAGE_ADDRESS (7)


void
ssd1306_init(ssd1306_t *device, uint8_t address)
{
  device->address = address;
  device->cursor_x = 0;
  device->cursor_page = 0;
}


void
ssd1306_start_update(ssd1306_t *device)
{
  device->mode = SSD1306_I2C_MODE_NOT_SELECTED;
}


void
ssd1306_switch_i2c_mode(ssd1306_t *device, ssd1306_i2c_mode_t mode)
{
  if (device->mode != mode) {

    if (device->mode != SSD1306_I2C_MODE_NOT_SELECTED) {
      i2c_async_send_repeated_start();
    }

    i2c_async_send_data(mode);
    device->mode = mode;
  }
}


void
ssd1306_send_command(ssd1306_t *device, uint8_t command_byte)
{
  ssd1306_switch_i2c_mode(device, SSD1306_I2C_MODE_COMMAND);
  i2c_async_send_data(command_byte);
}


void
ssd1306_send_data(ssd1306_t *device, uint8_t data_byte)
{
  ssd1306_switch_i2c_mode(device, SSD1306_I2C_MODE_DATA);
  i2c_async_send_data(data_byte);
}


void
ssd1306_move_to(ssd1306_t *device, uint8_t x, uint8_t page)
{
  ssd1306_send_command(device, SSD1306_CMD_SET_COLUMN_ADDRESS);
  ssd1306_send_command(device, x);
  ssd1306_send_command(device, SSD1306_MAX_COLUMN_ADDRESS);
  ssd1306_send_command(device, SSD1306_CMD_SET_PAGE_ADDRESS);
  ssd1306_send_command(device, page);
  ssd1306_send_command(device, SSD1306_MAX_PAGE_ADDRESS);
}


void
ssd1306_put_segment(ssd1306_t *device, uint8_t segment)
{
  ssd1306_send_data(device, segment);
}
