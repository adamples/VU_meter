#include "display.h"
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "assert.h"
#include "bitmap.h"


void
display_init(display_t *display, uint8_t address)
{
  display->sprites_n = 0;
  update_extents_reset(display->previous_extents, true);
  while (!oled_init(&(display->device), address));
}


void
display_add_sprite(display_t *display, sprite_t *sprite)
{
  assert(display->sprites_n < DISPLAY_MAX_SPRITES);

  display->sprites[display->sprites_n] = sprite;
  ++display->sprites_n;
}


static void
display_update_get_extents(display_t *display)
{
  update_extents_reset(display->current_extents, false);

  for (uint8_t i = 0; i < display->sprites_n; ++i) {
    if (display->sprites[i]->changed) {
      display->sprites[i]->add_to_extents(display->sprites[i], display->current_extents);
    }
  }

  for (uint8_t i = 0; i < EXTENTS_SIZE; ++i) {
    display->previous_extents[i] |= display->current_extents[i];
  }
}


static void
display_update_render(display_t *display, oled_draw_cmd_t *draw,
  uint8_t column, uint8_t page, uint8_t width)
{
  oled_draw_cmd_set_dimensions(draw, column, page, width);
  oled_segment_t *segments = oled_draw_cmd_get_segments(draw);

  #ifndef NDEBUG
    for (uint8_t i = 0; i < width; ++i) {
      segments[i] = 0x55 << (i & 1);
    }
  #endif

  for (uint8_t i = 0; i < display->sprites_n; ++i) {
    if (display->sprites[i]->visible) {
      display->sprites[i]->render(display->sprites[i], column, page, column + width - 1, segments);
    }
  }
}


void
display_force_full_update(display_t *display)
{
  update_extents_reset(display->previous_extents, true);
}


void
display_update(display_t *display)
{
  oled_draw_cmd_t buffer_a;
  oled_draw_cmd_t buffer_b;
  oled_draw_cmd_t *render_buffer = &buffer_a;
  oled_draw_cmd_t *transmit_buffer = &buffer_b;
  bool update_error = false;

  oled_draw_cmd_init(&buffer_a);
  oled_draw_cmd_init(&buffer_b);

  display_update_get_extents(display);

  uint8_t extents_word = 0;
  uint8_t extents_bit = 1;

  oled_draw_cmd_finish();

  for (uint8_t page = 0; page < OLED_PAGES_N; ++page)
  {
    for (uint8_t column = 0; column < OLED_COLUMNS_N; column += EXTENTS_TILE_WIDTH)
    {
      if (display->previous_extents[extents_word] & extents_bit)
      {
        display_update_render(display, render_buffer, column, page, EXTENTS_TILE_WIDTH);

        update_error |= !oled_draw_cmd_finish();

        oled_draw_cmd_t *tmp = transmit_buffer;
        transmit_buffer = render_buffer;
        render_buffer = tmp;

        oled_draw_cmd_start(transmit_buffer, &(display->device));
      }

      extents_bit <<= 1;

      if (extents_bit == 0)
      {
        ++extents_word;
        extents_bit = 1;
      }
    }
  }

  update_error |= !oled_draw_cmd_finish();

  if (update_error) {
    display_force_full_update(display);
  }
  else {
    memcpy(display->previous_extents, display->current_extents, EXTENTS_SIZE);
  }

  for (uint8_t i = 0; i < display->sprites_n; ++i) {
    display->sprites[i]->changed = false;
  }
}


void
update_extents_reset(update_extents_t extents, bool value)
{
  uint8_t fill_byte = (value) ? 0xff : 0x00;
  memset(extents, fill_byte, EXTENTS_SIZE);
}


#define EXTENTS_BITS_PER_PAGE (OLED_WIDTH / EXTENTS_TILE_WIDTH)

void
update_extents_add_page(update_extents_t extents, uint8_t page)
{
  uint8_t word = page * EXTENTS_BITS_PER_PAGE / 8;
  memset(&extents[word], 0xff, EXTENTS_BITS_PER_PAGE / 8);
}


void
update_extents_add_region(update_extents_t extents, uint8_t page, uint8_t start_column, uint8_t end_column)
{
  uint8_t start_bit = start_column / EXTENTS_TILE_WIDTH + page * EXTENTS_BITS_PER_PAGE;
  uint8_t end_bit = end_column / EXTENTS_TILE_WIDTH + page * EXTENTS_BITS_PER_PAGE;

  uint8_t word = start_bit / 8;
  uint8_t bit = 1 << (start_bit % 8);
  uint8_t bits_n = end_bit - start_bit + 1;

  for (uint8_t i = 0; i < bits_n; ++i) {
    extents[word] |= bit;
    bit <<= 1;

    if (bit == 0) {
      ++word;
      bit = 1;
    }
  }
}
