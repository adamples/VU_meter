#include "oled.h"
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "i2c.h"

#define OLED_MAX_COLUMN_ADDRESS (OLED_COLUMNS_N - 1)
#define OLED_MAX_PAGE_ADDRESS (OLED_PAGES_N - 1)


static const uint8_t OLED_INIT_SEQUENCE[] PROGMEM = {
    OLED_CTRL_COMMAND | OLED_CTRL_N_BYTES,
    OLED_CMD_SET_DISPLAY_ON(false),
    OLED_CMD_SET_MULTIPLEX_RATIO, 0x3F,
    OLED_CMD_SET_DISPLAY_OFFSET, 0x00,
    OLED_CMD_SET_DISPLAY_START_LINE(0x00),
    OLED_CMD_SET_SEGMENT_REMAP(true),
    OLED_CMD_SET_COM_SCAN_DIRECTION_DEC,
    OLED_CMD_SET_COM_PINS_HW_CONF, 0x12,
    OLED_CMD_SET_CONTRAST, 0x00,
    OLED_CMD_SET_ENTIRE_DISPLAY_ON(false),
    OLED_CMD_SET_INVERSED(false),
    OLED_CMD_SET_CLOCK_DIVIDE_FREQUENCY, 0xf0,
    OLED_CMD_SET_PRECHARGE_PERIOD, 0x2f,
    OLED_CMD_SET_VCOMH_DESELECT_LEVEL, 0x40,
    OLED_CMD_SET_DCDC_CONTROL_MODE, OLED_CMD_SET_DCDC_ON(true)
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
  draw->cmd_page = OLED_CMD_SET_PAGE_ADDRESS(draw->page);
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
