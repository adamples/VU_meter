#include "needle_sprite.h"
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "assert.h"
#include "needle_coordinates.h"
#include "ssd1306.h"


static void
draw_line_23_octants(int8_t buffer[], uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by)
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
needle_sprite_render_cb(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, ssd1306_segment_t* segments)
{
  needle_sprite_t *needle = (needle_sprite_t *) sprite;

  int8_t start_column = needle->start_column[page];
  int8_t end_column = needle->end_column[page];

  if (start_column == -128) {
    return;
  }

  start_column = int_max(start_column, column_a);
  end_column = int_min(end_column, column_b);

  for (uint8_t column = start_column; column <= end_column; ++column) {
    uint8_t segment = segments[column - column_a];

    for (uint8_t i = 0; i < 8; ++i) {
      uint8_t row = page * 8 + i;

      if (needle->column[row] < 0) {
        continue;
      }

      uint8_t bit = 1 << i;
      int8_t d = needle->column[row] - column;

      if (d < 0) d = -d;// - 1;

      if (d == 0) {
        segment |= bit;
      }
      else if (d <= 2) {
        segment &= ~bit;
      }
    }

    segments[column - column_a] = segment;
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
needle_sprite_draw(needle_sprite_t *needle, uint8_t angle)
{
  uint8_t index = (uint16_t) angle * NEEDLE_RESOLUTION / 256;
  uint8_t ax = pgm_read_byte(&(NEEDLE_COORDINATES[index].x));
  uint8_t ay = pgm_read_byte(&(NEEDLE_COORDINATES[index].y));

  draw_line_23_octants(needle->column, ax, ay, NEEDLE_AXIS_X, NEEDLE_AXIS_Y);

  uint8_t page = 0;

  /* Pages over the needle */
  for (; page < ay / SSD1306_PAGE_HEIGHT; ++page) {
    needle->start_column[page] = -128;
    needle->end_column[page] = -128;
  }

  /* Page, where the needle has it's tip */
  if (ax <= NEEDLE_AXIS_X) {
    needle->start_column[page] = ax;
    needle->end_column[page] = needle->column[(page + 1) * SSD1306_PAGE_HEIGHT - 1];
  }
  else {
    needle->start_column[page] = needle->column[(page + 1) * SSD1306_PAGE_HEIGHT - 1];
    needle->end_column[page] = ax;
  }

  ++page;

  /* Pages below the needle tip */
  for (; page < SSD1306_PAGES_N; ++page) {
    int8_t top_x = needle->column[page * SSD1306_PAGE_HEIGHT];
    int8_t bottom_x = needle->column[(page + 1) * SSD1306_PAGE_HEIGHT - 1];
    needle->start_column[page] = int_min(top_x, bottom_x);
    needle->end_column[page] = int_max(top_x, bottom_x);
  }

  /* Take account of needle shadow width */
  for (page = 0; page < SSD1306_PAGES_N; ++page) {
    if (needle->start_column[page] == -128) {
      continue;
    }

    if (needle->start_column[page] <= 3) {
      needle->start_column[page] = 0;
    }
    else {
      needle->start_column[page] -= 3;
    }

    if (needle->end_column[page] >= SSD1306_WIDTH - 3) {
      needle->end_column[page] = SSD1306_WIDTH - 1;
    }
    else {
      needle->end_column[page] += 3;
    }
  }

#ifndef NDEBUG
  for (page = 0; page < SSD1306_PAGES_N; ++page) {
    assert(needle->start_column[page] <= needle->end_column[page]);
  }
#endif
}


void
needle_sprite_add_to_extents(needle_sprite_t *needle, update_extents_t *extents)
{
  for (uint8_t page = 0; page < SSD1306_PAGES_N; ++page) {
    if (needle->start_column[page] == -128) {
      continue;
    }

    update_extents_add_region(extents, page, needle->start_column[page], needle->end_column[page]);
  }
}
