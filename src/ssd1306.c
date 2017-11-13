#include "ssd1306.h"

#define SSD1306_MAX_COLUMN_ADDRESS (127)
#define SSD1306_MAX_PAGE_ADDRESS (7)


struct ssd1306_t_ {
  uint8_t address;
  uint8_t cursor_x;
  uint8_t cursor_page;
  ssd1306_i2c_mode_t mode;
};


void
ssd1306_init(ssd1306_t *display, uint8_t address)
{
  display->address = address;
  display->cursor_x = 0;
  display->cursor_page = 0;
}


void
ssd1306_switch_i2c_mode(ssd1306_t *display, ssd1306_i2c_mode_t mode)
{
  if (display->mode != mode) {

    if (display->mode != SSD1306_I2C_MODE_NOT_SELECTED) {
      ssd1306_buffer_push_i2c_command(I2C_COMMAND_REPEATED_START);
    }

    ssd1306_buffer_push_i2c_data(mode);
    display->mode = SSD1306_MODE_COMMAND;
  }
}


void
ssd1306_send_command(ssd1306_t *display, uint8_t command_byte)
{
  ssd1306_switch_i2c_mode(display, SSD1306_I2C_MODE_COMMAND);
  ssd1306_buffer_push_i2c_data(command_byte);
}


void
ssd1306_send_data(ssd1306_t *display, uint8_t data_byte)
{
  ssd1306_switch_i2c_mode(display, SSD1306_I2C_MODE_DATA);
  ssd1306_buffer_push_i2c_data(command_byte);
}


void
ssd1306_move_to(ssd1306_t *display, uint8_t x, uint8_t page)
{
  ssd1306_send_command(display, SSD1306_CMD_SET_COLUMN_ADDRESS);
  ssd1306_send_command(display, x);
  ssd1306_send_command(display, SSD1306_MAX_COLUMN_ADDRESS);
  ssd1306_send_command(display, SSD1306_CMD_SET_PAGE_ADDRESS);
  ssd1306_send_command(display, page);
  ssd1306_send_command(display, SSD1306_MAX_PAGE_ADDRESS);
}


void
ssd1306_put_segment(ssd1306_t *display, uint8_t segment)
{
  ssd1306_send_data(display, segment);
}
