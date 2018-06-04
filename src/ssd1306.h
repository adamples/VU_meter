#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_COLUMNS_N (128)
#define OLED_PAGES_N (8)
#define OLED_PAGE_HEIGHT (8)

#define OLED_WIDTH (OLED_COLUMNS_N)
#define OLED_HEIGHT (OLED_PAGES_N * OLED_PAGE_HEIGHT)

#define OLED_CMD_SET_LOW_COLUMN(low) (0x00 | low)
#define OLED_CMD_SET_HIGH_COLUMN(hi) (0x10 | hi)
#define OLED_CMD_SET_MEMORY_ADDRESSING_MODE (0x20)
#define OLED_CMD_SET_COLUMN_ADDRESS (0x21)
#define OLED_CMD_SET_PAGE_ADDRESS (0x22)
#define OLED_CMD_SET_DISPLAY_START_LINE(line) (0x40 | line)
#define OLED_CMD_SET_CONTRAST (0x81)
#define OLED_CMD_CHARGE_PUMP_SETTING (0x8D)
#define OLED_CMD_SET_SEGMENT_REMAP(is_enabled) (0xA0 | is_enabled)
#define OLED_CMD_ENTIRE_DISPLAY_ON(is_on) (0xA4 | is_on)
#define OLED_CMD_SET_INVERSED(is_inversed) (0xA6 | is_inversed)
#define OLED_CMD_SET_MULTIPLEX_RATIO (0xA8)
#define OLED_CMD_SET_DISPLAY_ON(is_on) (0xAE | is_on)
#define OLED_CMD_SET_PAGE_START_ADDRESS(address) (0xB0 | address)
#define OLED_CMD_SET_COM_SCAN_DIRECTION_INC (0xC0)
#define OLED_CMD_SET_COM_SCAN_DIRECTION_DEC (0xC8)
#define OLED_CMD_SET_DISPLAY_OFFSET (0xD3)
#define OLED_CMD_SET_CLOCK_DIVIDE_FREQUENCY (0xD5)
#define OLED_CMD_SET_PRECHARGE_PERIOD (0xD9)
#define OLED_CMD_SET_COM_PINS_HW_CONF (0xDA)
#define OLED_CMD_SET_VCOMH_DESELECT_LEVEL (0xDB)
#define OLED_CMD_NOP (0xE3)


typedef uint8_t oled_segment_t;

typedef enum oled_i2c_mode_t_ {
  OLED_I2C_MODE_COMMAND = 0x00,
  OLED_I2C_MODE_DATA = 0x40,
  OLED_I2C_MODE_NOT_SELECTED = 0xff
} oled_i2c_mode_t;

typedef struct oled_t_ {
  uint8_t address;
  uint8_t cursor_column;
  uint8_t cursor_page;
  oled_i2c_mode_t i2c_mode;
} oled_t;

typedef bool (*oled_update_callback_t)(void *data);


void oled_init(oled_t *device, uint8_t address);

void oled_start_update(oled_t *device, oled_update_callback_t callback, void *data);

void oled_move_to(oled_t *device, uint8_t column, uint8_t page);
void oled_put_segments(oled_t *device, uint8_t column, uint8_t page,
                          uint8_t width, uint8_t *segments);

void oled_finish_update(oled_t *device);


#endif /* OLED_H */
