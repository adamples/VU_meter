#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_COLUMNS_N (128)
#define OLED_COLUMN_OFFSET (0)
#define OLED_PAGES_N (8)
#define OLED_PAGE_HEIGHT (8)

#define OLED_WIDTH (OLED_COLUMNS_N)
#define OLED_HEIGHT (OLED_PAGES_N * OLED_PAGE_HEIGHT)

#define OLED_CTRL_COMMAND (0x00)
#define OLED_CTRL_DATA (0x40)
#define OLED_CTRL_N_BYTES (0x00)
#define OLED_CTRL_SINGLE_BYTE (0x80)

#define OLED_MEMORY_ADDRESSING_MODE_PAGE (0b10)
#define OLED_MEMORY_ADDRESSING_MODE_HORIZONTAL (0b00)
#define OLED_MEMORY_ADDRESSING_MODE_VERTICAL (0b01)

#define OLED_CMD_SET_LOW_COLUMN(low) (0x00 | (low))
#define OLED_CMD_SET_LOW_COLUMN(low) (0x00 | (low))
#define OLED_CMD_SET_HIGH_COLUMN(hi) (0x10 | (hi))
#define OLED_CMD_SET_MEMORY_ADDRESSING_MODE (0x20)
#define OLED_CMD_SET_COLUMN_ADDRESS (0x21)
#define OLED_CMD_SET_PAGE_ADDRESS (0x22)
#define OLED_CMD_DEACTIVATE_SCROLL (0x2e)
#define OLED_CMD_SET_DISPLAY_START_LINE(line) (0x40 | (line))
#define OLED_CMD_SET_CONTRAST (0x81)
#define OLED_CMD_CHARGE_PUMP_SETTING (0x8D)
#define OLED_CMD_SET_SEGMENT_REMAP(is_enabled) (0xA0 | (is_enabled))
#define OLED_CMD_ENTIRE_DISPLAY_ON(is_on) (0xA4 | (is_on))
#define OLED_CMD_SET_INVERSED(is_inversed) (0xA6 | (is_inversed))
#define OLED_CMD_SET_MULTIPLEX_RATIO (0xA8)
#define OLED_CMD_SET_DISPLAY_ON(is_on) (0xAE | (is_on))
#define OLED_CMD_SET_PAGE_START_ADDRESS(address) (0xB0 | (address))
#define OLED_CMD_SET_COM_SCAN_DIRECTION_INC (0xC0)
#define OLED_CMD_SET_COM_SCAN_DIRECTION_DEC (0xC8)
#define OLED_CMD_SET_DISPLAY_OFFSET (0xD3)
#define OLED_CMD_SET_CLOCK_DIVIDE_FREQUENCY (0xD5)
#define OLED_CMD_SET_PRECHARGE_PERIOD (0xD9)
#define OLED_CMD_SET_COM_PINS_HW_CONF (0xDA)
#define OLED_CMD_SET_VCOMH_DESELECT_LEVEL (0xDB)
#define OLED_CMD_NOP (0xE3)


typedef uint8_t oled_segment_t;

typedef struct oled_t_ {
  uint8_t address;
  uint8_t column;
  uint8_t page;
} oled_t;


typedef struct oled_draw_cmd_t_ {
  uint8_t column;
  uint8_t page;
  uint8_t width;
  uint8_t address;
  uint8_t ctrl1;
  uint8_t cmd_high_column;
  uint8_t ctrl2;
  uint8_t cmd_low_column;
  uint8_t ctrl3;
  uint8_t cmd_page;
  uint8_t ctrl4;
  uint8_t gddram_data[OLED_DRAW_BUFFER_SIZE];
} oled_draw_cmd_t;


void oled_draw_cmd_init(oled_draw_cmd_t *draw);
void oled_draw_cmd_set_address(oled_draw_cmd_t *draw, uint8_t address);
void oled_draw_cmd_set_dimensions(oled_draw_cmd_t *draw, uint8_t column, uint8_t page, uint8_t width);
oled_segment_t *oled_draw_cmd_get_segments(oled_draw_cmd_t *draw);

bool oled_init(oled_t *device, uint8_t address);
void oled_set_display_on(oled_t *device, bool enabled);

void oled_draw_cmd_start(oled_draw_cmd_t *draw, oled_t *device);
bool oled_draw_cmd_finish();


#endif /* OLED_H */
