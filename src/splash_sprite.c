#include "splash_sprite.h"
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "assert.h"


#define ANIMATION_OFFSET (128)
#define ANIMATION_FRAMES (24)


static int8_t
splash_frame_to_position(splash_sprite_t *splash)
{
  return (splash->frame_idx - ANIMATION_OFFSET) * 32 / ANIMATION_FRAMES;
}


static bool
splash_animation_active(splash_sprite_t *splash)
{
  return (splash->frame_idx >= ANIMATION_OFFSET &&
    splash->frame_idx < ANIMATION_OFFSET + ANIMATION_FRAMES);
}


static void
splash_sprite_render(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, oled_segment_t* segments)
{
  splash_sprite_t *splash = (splash_sprite_t *) sprite;

  if (splash->finished) {
    return;
  }

  const uint8_t *source = splash->data + column_a + page * OLED_WIDTH;
  oled_segment_t *target = segments;

  int8_t position = -1;
  uint8_t line = 0x00;
  uint8_t mask = 0xff;
  uint8_t page_start;

  if (splash_animation_active(splash))
  {
    position = splash_frame_to_position(splash);
  }

  if (position >= 0) {
    if (page < OLED_PAGES_N / 2) {
      page_start = 24 - page * 8;
    }
    else {
      page_start = page * 8 - 32;
    }

    if (page_start > position) {
      mask = 0xff;
    }
    else if (page_start + 8 < position) {
      mask = 0x00;
    }
    else {
      if (page < OLED_PAGES_N / 2) {
        mask = 0xff >> (position - page_start);
        line = 0x80 >> (position - page_start);
      }
      else {
        mask = 0xff << (position - page_start);
        line = 0x01 << (position - page_start);
      }
    }
  }

  for (uint8_t i = column_a; i <= column_b; ++i) {
    uint8_t image = pgm_read_byte(source);
    *target = (*target & ~mask) | (image & mask) | line;
    ++target;
    ++source;
  }
}


void
splash_sprite_init(splash_sprite_t *splash, const uint8_t *data)
{
  splash->sprite.render = &splash_sprite_render;
  splash->sprite.add_to_extents = (sprite_add_to_extents_t) &splash_sprite_add_to_extents;
  splash->sprite.visible = true;
  splash->sprite.changed = true;
  splash->data = data + 2;
  splash->frame_idx = 0;
  splash->finished = false;
}


void
splash_sprite_advance(splash_sprite_t *splash)
{
  if (!splash->finished) {
    splash->sprite.changed = true;
    ++splash->frame_idx;
    splash->finished = splash->frame_idx >= ANIMATION_OFFSET + ANIMATION_FRAMES;
  }
}


bool
splash_sprite_is_finished(splash_sprite_t *splash)
{
  return (splash->finished == true);
}


void
splash_sprite_add_to_extents(splash_sprite_t *splash, update_extents_t extents)
{
  if (splash_animation_active(splash)) {
    int8_t position = splash_frame_to_position(splash);
    uint8_t page_a = OLED_PAGES_N / 2 + position / OLED_PAGE_HEIGHT;
    uint8_t page_b = OLED_PAGES_N / 2 - 1 - position / OLED_PAGE_HEIGHT;
    update_extents_add_page(extents, page_a);
    update_extents_add_page(extents, page_b);
  }
}
