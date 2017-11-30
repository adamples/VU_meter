#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "ssd1306.h"

#define DISPLAY_MAX_SPRITES (4)

typedef struct segment_t_ {
  uint8_t reserved;
  uint8_t value;
} segment_t;

typedef struct sprite_t_ sprite_t;

typedef void (*sprite_get_extent_t)(sprite_t *sprite, uint8_t page, uint8_t *min, uint8_t *max);
typedef void (*sprite_render_t)(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, segment_t* segments);

struct sprite_t_ {
  sprite_get_extent_t get_extent;
  sprite_render_t render;
  bool visible;
};


typedef struct display_t_ {
  ssd1306_t *device;
  sprite_t *sprites[DISPLAY_MAX_SPRITES];
  uint8_t sprites_n;
  /* uint8_t render_extents[SSD1306_HEIGHT][2]; */
  uint8_t update_column, update_page;
} display_t;


void display_init(display_t *display, ssd1306_t *device);
void display_add_sprite(display_t *display, sprite_t *sprite);
void display_update_async(display_t *display);


typedef struct progmem_image_sprite_t_ {
  sprite_t sprite;
  uint8_t column;
  uint8_t page;
  uint8_t width;
  uint8_t height;
  const uint8_t *data;
} progmem_image_sprite_t;


void progmem_image_sprite_init(progmem_image_sprite_t *image,
  const uint8_t *data, uint8_t column, uint8_t page);

#endif /* DISPLAY_H */
