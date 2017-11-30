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

#define SEGMENTS_N (16)

bool
display_update_async_cb(i2c_command_buffer_t *commands, display_t *display)
{
  //~ uint8_t segments[SEGMENTS_N];

  //~ for (uint8_t i = 0; i < I2C_BUFFER_SIZE; ++i) {
    //~ commands->commands[i].code = 0xff;
    //~ commands->commands[i].data = 0x00;
  //~ }

  if (display->update_column == 0 && display->update_page == 0) {
    //~ ssd1306_start_update(display->device);
    //~ ssd1306_move_to(display->device, 0, 0);
    //~ ssd1306_switch_i2c_mode(display->device, SSD1306_I2C_MODE_DATA);
    //~ i2c_async_send_start();
    //~ i2c_async_send_data(0x40);
    i2c_command_buffer_append_start(commands);
    i2c_command_buffer_append_send_data(commands, display->device->address);
    i2c_command_buffer_append_send_data(commands, 0x00);
    i2c_command_buffer_append_send_data(commands, SSD1306_CMD_SET_COLUMN_ADDRESS);
    i2c_command_buffer_append_send_data(commands, 0);
    i2c_command_buffer_append_send_data(commands, 127);
    i2c_command_buffer_append_send_data(commands, SSD1306_CMD_SET_PAGE_ADDRESS);
    i2c_command_buffer_append_send_data(commands, 0);
    i2c_command_buffer_append_send_data(commands, 7);
    i2c_command_buffer_append_start(commands);
    i2c_command_buffer_append_send_data(commands, display->device->address);
    i2c_command_buffer_append_send_data(commands, 0x40);
  }

  for (uint8_t i = 0; i < SEGMENTS_N; ++i) {
    commands->commands[commands->length + i].code = I2C_COMMAND_SEND_DATA;
    commands->commands[commands->length + i].data = 0;
  }

  for (uint8_t i = 0; i < display->sprites_n; ++i) {
    if (display->sprites[i]->visible) {
      display->sprites[i]->render(
        display->sprites[i],
        display->update_column,
        display->update_page,
        display->update_column + SEGMENTS_N - 1,
        (segment_t *) &(commands->commands[commands->length])
      );
    }
  }

  commands->length += SEGMENTS_N;

  //~ for (uint8_t i = 0; i < SEGMENTS_N; ++i)
    //~ i2c_async_send_data(segments[i]);
    //~ i2c_command_buffer_append_send_data(commands, segments[i]);
    //~ ssd1306_put_segment(display->device, segments[i]);

  //~ ssd1306_put_segments(display->device, segments, SEGMENTS_N);

  display->update_column += SEGMENTS_N;

  if (display->update_column >= SSD1306_COLUMNS_N) {
    display->update_column = 0;
    ++display->update_page;

    if (display->update_page == SSD1306_PAGES_N) {
      i2c_async_end_transmission();
      return false;
    }
  }

  return true;
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
progmem_image_sprite_render(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, segment_t* segments)
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
  segment_t *target = segments + target_column_a - column_a;

  for (uint8_t i = target_column_a; i <= target_column_b; ++i) {
    target->value = pgm_read_byte(source);
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
