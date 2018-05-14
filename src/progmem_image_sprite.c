#include "progmem_image_sprite.h"
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "assert.h"


static void
progmem_image_sprite_render(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, ssd1306_segment_t* segments)
{
  progmem_image_sprite_t *image = (progmem_image_sprite_t *) sprite;

  if (column_b < image->column || column_a > image->column + image->width ||
    page < image->page || page >= image->page + image->height)
  {
    return;
  }

  int8_t target_column_a = int_max(column_a, image->column);
  int8_t target_column_b = int_min(column_b, image->column + image->width);
  int8_t source_column_a = target_column_a - image->column;
  int8_t source_page = page - image->page;

  const uint8_t *source = image->data + source_column_a + source_page * image->width;
  ssd1306_segment_t *target = segments + target_column_a - column_a;

  for (uint8_t i = target_column_a; i <= target_column_b; ++i) {
    *target = pgm_read_byte(source);
    ++target;
    ++source;
  }
}


void
progmem_image_sprite_init(progmem_image_sprite_t *image, const uint8_t *data, uint8_t column, uint8_t page)
{
  image->sprite.render = &progmem_image_sprite_render;
  image->sprite.visible = true;
  image->column = column;
  image->page = page;
  image->width = pgm_read_byte(data);
  image->height = pgm_read_byte(data + 1) / SSD1306_PAGE_HEIGHT;
  image->data = data + 2;
}


void
progmem_image_sprite_add_to_extents(progmem_image_sprite_t *image, update_extents_t *extents)
{
  for (uint8_t page = image->page; page < image->page + image->height; ++page)
    update_extents_add_region(extents, page, image->column, image->column + image->width - 1);
}
