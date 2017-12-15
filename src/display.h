#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "ssd1306.h"

#define DISPLAY_MAX_SPRITES (4)


typedef struct sprite_t_ sprite_t;

typedef void (*sprite_get_extent_t)(sprite_t *sprite, uint8_t page, uint8_t *min, uint8_t *max);
typedef void (*sprite_render_t)(sprite_t *sprite, uint8_t column_a, uint8_t page, uint8_t column_b, ssd1306_segment_t* segments);

struct sprite_t_ {
  sprite_get_extent_t get_extent;
  sprite_render_t render;
  bool visible;
};


typedef struct region_t_ {
  uint8_t page;
  uint8_t start_column;
  uint8_t end_column;
} region_t;

typedef struct update_extents_t_ {
  uint8_t regions_n;
  region_t *regions;
} update_extents_t;

typedef struct partial_update_ctrl_t_ {
  uint8_t region_index;
  uint8_t column;
  update_extents_t *extents;
} partial_update_ctrl_t;


typedef struct full_update_ctrl_t_ {
  uint8_t page;
  uint8_t column;
} full_update_ctrl_t;


typedef union update_ctrl_t_ {
  partial_update_ctrl_t partial;
  full_update_ctrl_t full;
} update_ctrl_t;


typedef struct display_t_ {
  ssd1306_t *device;
  sprite_t *sprites[DISPLAY_MAX_SPRITES];
  uint8_t sprites_n;
  update_ctrl_t update;
} display_t;


void display_init(display_t *display, ssd1306_t *device);
void display_add_sprite(display_t *display, sprite_t *sprite);
void display_update_async(display_t *display);
void display_update_partial_async(display_t *display, update_extents_t *extents);

#endif /* DISPLAY_H */
