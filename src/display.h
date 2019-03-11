#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "oled.h"

#define DISPLAY_MAX_SPRITES (4)

#define EXTENTS_TILE_WIDTH (8)
#define EXTENTS_TILE_HEIGHT (8)
#define EXTENTS_SIZE (OLED_WIDTH * OLED_HEIGHT / EXTENTS_TILE_HEIGHT / EXTENTS_TILE_WIDTH / 8)

typedef uint8_t update_extents_t[EXTENTS_SIZE];

typedef struct sprite_t_ sprite_t;
typedef struct display_t_ display_t;

typedef void (*sprite_render_t)(
  sprite_t *sprite,
  uint8_t column_a,
  uint8_t page,
  uint8_t column_b,
  oled_segment_t* segments
);

typedef void (*sprite_add_to_extents_t)(
  sprite_t *sprite,
  update_extents_t extents
);

struct sprite_t_ {
  sprite_render_t render;
  sprite_add_to_extents_t add_to_extents;
  bool visible;
  bool changed;
};

struct display_t_ {
  oled_t device;
  sprite_t *sprites[DISPLAY_MAX_SPRITES];
  uint8_t sprites_n;
  update_extents_t previous_extents;
  update_extents_t current_extents;
};


void display_init(display_t *display, uint8_t address);
void display_add_sprite(display_t *display, sprite_t *sprite);
void display_force_full_update(display_t *display);
void display_update(display_t *display);

void update_extents_reset(update_extents_t extents, bool value);
void update_extents_add_page(update_extents_t extents, uint8_t page);
void update_extents_add_region(update_extents_t extents, uint8_t page, uint8_t start_column, uint8_t end_column);

#endif /* DISPLAY_H */
