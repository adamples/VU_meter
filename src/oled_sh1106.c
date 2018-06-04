#include "oled.h"
#include <avr/pgmspace.h>
#include "i2c.h"

#define OLED_MAX_COLUMN_ADDRESS (OLED_COLUMNS_N - 1)
#define OLED_MAX_PAGE_ADDRESS (OLED_PAGES_N - 1)


static const uint8_t OLED_INIT_SEQUENCE[] PROGMEM = {
    0x00,
    OLED_CMD_SET_DISPLAY_ON(false),
    OLED_CMD_SET_MULTIPLEX_RATIO, 0x3F,
    OLED_CMD_SET_DISPLAY_OFFSET, 0x00,
    OLED_CMD_SET_DISPLAY_START_LINE(0x00),
    //~ OLED_CMD_SET_MEMORY_ADDRESSING_MODE, 0x00,
    OLED_CMD_SET_SEGMENT_REMAP(true), OLED_CMD_SET_COM_SCAN_DIRECTION_DEC,
    /* rotated: OLED_CMD_SET_SEGMENT_REMAP(false), OLED_CMD_SET_COM_SCAN_DIRECTION_INC */
    OLED_CMD_SET_COM_PINS_HW_CONF, 0x12,
    OLED_CMD_SET_PRECHARGE_PERIOD, 0x11,
    OLED_CMD_SET_VCOMH_DESELECT_LEVEL, 0x40,
    OLED_CMD_SET_ENTIRE_DISPLAY_ON(false),
    OLED_CMD_SET_INVERSED(false),
    OLED_CMD_SET_CLOCK_DIVIDE_FREQUENCY, 0xf0,
    OLED_CMD_SET_CONTRAST, 0xcf,
    OLED_CMD_SET_DCDC_CONTROL_MODE, OLED_CMD_SET_DCDC_ON(true),
    OLED_CMD_SET_DISPLAY_ON(true)
};


void
oled_init(oled_t *device, uint8_t address)
{
  device->address = address;
  device->cursor_column = 255; /* Force column / page addressing */
  device->cursor_page = 255;
  device->i2c_mode = OLED_I2C_MODE_NOT_SELECTED;
  i2c_transmit_progmem(device->address, OLED_INIT_SEQUENCE, sizeof(OLED_INIT_SEQUENCE));
}


void
oled_start_update(oled_t *device, oled_update_callback_t callback, void *data)
{
  i2c_transmit_async(device->address, callback, data);
}


void
oled_finish_update(oled_t *device)
{
  device->i2c_mode = OLED_I2C_MODE_NOT_SELECTED;
  i2c_async_end_transmission();
}


static void
oled_switch_i2c_mode(oled_t *device, oled_i2c_mode_t i2c_mode)
{
  if (device->i2c_mode != i2c_mode) {
    i2c_async_send_start();
    i2c_async_send_byte(i2c_mode);
    device->i2c_mode = i2c_mode;
  }
}


void
oled_move_to(oled_t *device, uint8_t column, uint8_t page)
{
  if (device->cursor_column != column || device->cursor_page != page) {
    oled_switch_i2c_mode(device, OLED_I2C_MODE_COMMAND);

    column += OLED_COLUMN_OFFSET;

    i2c_async_send_byte(OLED_CMD_SET_LOW_COLUMN(column & 0x0f));
    i2c_async_send_byte(OLED_CMD_SET_HIGH_COLUMN(column >> 4));
    i2c_async_send_byte(OLED_CMD_SET_PAGE_ADDRESS(page));

    device->cursor_column = column;
    device->cursor_page = page;
  }
}


void
oled_write_gddram(oled_t *device, uint8_t length, uint8_t *data)
{
  oled_switch_i2c_mode(device, OLED_I2C_MODE_DATA);
  i2c_async_send_bytes(data, length);

  device->cursor_column += length;
}


void
oled_put_segments(oled_t *device, uint8_t column, uint8_t page, uint8_t width, uint8_t *segments)
{
  oled_move_to(device, column, page);
  oled_write_gddram(device, width, segments);
}
