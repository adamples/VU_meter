#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

#define SSD1306_COLUMNS_N (128)
#define SSD1306_PAGES_N (8)
#define SSD1306_PAGE_HEIGHT (8)

#define SSD1306_WIDTH (SSD1306_COLUMNS_N)
#define SSD1306_HEIGHT (SSD1306_PAGES_N * SSD1306_PAGE_HEIGHT)

#define SSD1306_CMD_SET_LOW_COLUMN(low) (0x00 | low)
#define SSD1306_CMD_SET_HIGH_COLUMN(hi) (0x10 | hi)
#define SSD1306_CMD_SET_MEMORY_ADDRESSING_MODE (0x20)
#define SSD1306_CMD_SET_COLUMN_ADDRESS (0x21)
#define SSD1306_CMD_SET_PAGE_ADDRESS (0x22)
#define SSD1306_CMD_SET_DISPLAY_START_LINE(line) (0x40 | line)
#define SSD1306_CMD_SET_CONTRAST (0x81)
#define SSD1306_CMD_CHARGE_PUMP_SETTING (0x8D)
#define SSD1306_CMD_SET_SEGMENT_REMAP(is_enabled) (0xA0 | is_enabled)
#define SSD1306_CMD_ENTIRE_DISPLAY_ON(is_on) (0xA4 | is_on)
#define SSD1306_CMD_SET_INVERSED(is_inversed) (0xA6 | is_inversed)
#define SSD1306_CMD_SET_MULTIPLEX_RATIO (0xA8)
#define SSD1306_CMD_SET_DISPLAY_ON(is_on) (0xAE | is_on)
#define SSD1306_CMD_SET_PAGE_START_ADDRESS(address) (0xB0 | address)
#define SSD1306_CMD_SET_COM_SCAN_DIRECTION_INC (0xC0)
#define SSD1306_CMD_SET_COM_SCAN_DIRECTION_DEC (0xC8)
#define SSD1306_CMD_SET_DISPLAY_OFFSET (0xD3)
#define SSD1306_CMD_SET_CLOCK_DIVIDE_FREQUENCY (0xD5)
#define SSD1306_CMD_SET_PRECHARGE_PERIOD (0xD9)
#define SSD1306_CMD_SET_COM_PINS_HW_CONF (0xDA)
#define SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL (0xDB)
#define SSD1306_CMD_NOP (0xE3)


typedef uint8_t ssd1306_segment_t;

typedef enum ssd1306_i2c_mode_t_ {
  SSD1306_I2C_MODE_COMMAND = 0x00,
  SSD1306_I2C_MODE_DATA = 0x40,
  SSD1306_I2C_MODE_NOT_SELECTED = 0xff
} ssd1306_i2c_mode_t;

typedef struct ssd1306_t_ {
  uint8_t address;
  uint8_t cursor_column;
  uint8_t cursor_page;
  ssd1306_i2c_mode_t i2c_mode;
} ssd1306_t;

typedef bool (*ssd1306_update_callback_t)(void *data);


void ssd1306_init(ssd1306_t *device, uint8_t address);

void ssd1306_start_update(ssd1306_t *device, ssd1306_update_callback_t callback, void *data);

void ssd1306_move_to(ssd1306_t *device, uint8_t column, uint8_t page);
void ssd1306_put_segments(ssd1306_t *device, uint8_t column, uint8_t page,
                          uint8_t width, uint8_t *segments);

void ssd1306_finish_update(ssd1306_t *device);


#endif /* SSD1306_H */
