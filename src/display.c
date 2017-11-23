#include "display.h"
#include <avr/pgmspace.h>
#include "assert.h"


void
display_init(display_t *display, ssd1306_t *device)
{
  display->device = device;
  display->sprites_n = 0;
}


void
display_add_sprite(display_t *display, sprite_t *sprite)
{
  assert(display->sprites_n < DISPLAY_MAX_SPRITES);

  display->sprites[display->sprites_n] = sprite;
  ++display->sprites_n;
}

#define SEGMENTS_N (32)

void
display_update_async_cb(display_t *display)
{
  uint8_t segments[SEGMENTS_N] = { 0 };

  if (display->update_column == 0 && display->update_page == 0) {
    ssd1306_start_update(display->device);
    ssd1306_move_to(display->device, 0, 0);
    i2c_async_send_repeated_start();
    i2c_async_send_data(0x40);
  }

  for (uint8_t i = 0; i < display->sprites_n; ++i) {
    if (display->sprites[i]->visible) {
      display->sprites[i]->render(
        display->sprites[i],
        display->update_column,
        display->update_page,
        display->update_column + SEGMENTS_N - 1,
        segments
      );
    }
  }

  for (uint8_t i = 0; i < SEGMENTS_N; ++i)
    i2c_async_send_data(segments[i]);
    //~ ssd1306_put_segment(display->device, segments[i]);

  display->update_column += SEGMENTS_N;

  if (display->update_column >= SSD1306_COLUMNS_N) {
    display->update_column = 0;
    ++display->update_page;

    if (display->update_page == SSD1306_PAGES_N) {
      i2c_async_end_transmission();
    }
  }
}


void
display_update_async(display_t *display)
{
  display->update_column = 0;
  display->update_page = 0;

  i2c_transmit_async(
    display->device->address,
    (i2c_callback_t) display_update_async_cb,
    display
  );
}


#define int_min(a, b) (((a) < (b)) ? (a) : (b))
#define int_max(a, b) (((a) > (b)) ? (a) : (b))

static void
progmem_image_sprite_render(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, uint8_t* segments)
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
  uint8_t *target = segments + target_column_a - column_a;

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
  image->height = pgm_read_byte(data + 1) / SSD1306_SEGMENT_HEIGHT;
  image->data = data + 2;
}
