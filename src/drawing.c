#include "drawing.h"
#include "assert.h"
#include <stdlib.h>
#include <string.h>


void draw_line_23_octants(int8_t buffer[], uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by)
{
  int8_t error = 0;
  int8_t delta_err = abs(bx - ax);
  int8_t error_threshold = (by - ay) / 2;
  int8_t delta_x = (bx > ax) ? 1 : -1;
  int8_t y;

  for (y = 0; y < ay; ++y) {
    buffer[y] = -128;
  }

  for (int8_t y = ay, x = ax; y <= by && y < SSD1306_HEIGHT; ++y) {
    buffer[y] = x;
    error += delta_err;

    if (error >= error_threshold) {
      x += delta_x;
      error -= error_threshold * 2;
    }
  }

  for (y = by + 1; y < SSD1306_HEIGHT; ++y) {
    buffer[y] = -128;
  }
}


static void
needle_sprite_render_cb(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, uint8_t* segments)
{
  needle_sprite_t *needle = (needle_sprite_t *) sprite;

  for (uint8_t column = column_a; column <= column_b; ++column) {

    uint8_t result = *segments;

    for (uint8_t i = 0; i < 8; ++i) {
      uint8_t row = page * 8 + i;

      if (needle->column[row] < 0) {
        continue;
      }

      uint8_t bit = 1 << i;
      int8_t d = needle->column[row] - column;

      if (d < 0) d = -d;// - 1;

      if (d == 0) {
        result |= bit;
      }
      else if (d <= 2) {
        result &= ~bit;
      }
    }

    *segments = result;
    ++segments;
  }
}


void
needle_sprite_init(needle_sprite_t *needle)
{
  needle->sprite.render = needle_sprite_render_cb;
  needle->sprite.visible = true;
  memset(needle->column, -128, sizeof(needle->column));
}


void
needle_sprite_draw(needle_sprite_t *needle, uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by)
{
  draw_line_23_octants(needle->column, ax, ay, bx, by);
}
