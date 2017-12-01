#include "display.h"
#include <avr/pgmspace.h>
#include "assert.h"


void
display_init(display_t *display, ssd1306_t *device)
{
  display->device = device;
  display->sprites_n = 0;
  display->update_column = 0;
  display->update_page = 0;
}


void
display_add_sprite(display_t *display, sprite_t *sprite)
{
  assert(display->sprites_n < DISPLAY_MAX_SPRITES);

  display->sprites[display->sprites_n] = sprite;
  ++display->sprites_n;
}

#define SEGMENTS_N (32)

bool
display_update_async_cb(display_t *display)
{
  ssd1306_segment_t segments[SEGMENTS_N];

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

  ssd1306_put_segments(
    display->device,
    display->update_column,
    display->update_page,
    SEGMENTS_N,
    segments
  );

  display->update_column += SEGMENTS_N;

  if (display->update_column >= SSD1306_COLUMNS_N) {
    display->update_column = 0;
    ++display->update_page;

    if (display->update_page >= SSD1306_PAGES_N) {
      display->update_column = 0;
      display->update_page = 0;
      ssd1306_finish_update(display->device);
      return false;
    }
  }

  return true;
}


void
display_update_async(display_t *display)
{
  ssd1306_start_update(
    display->device,
    (ssd1306_update_callback_t) display_update_async_cb,
    display
  );
}
