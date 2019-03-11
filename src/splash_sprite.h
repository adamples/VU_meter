#ifndef SPLASH_SPRITE_H
#define SPLASH_SPRITE_H

#include <stdint.h>
#include "oled.h"
#include "display.h"


typedef struct splash_sprite_t_ {
  sprite_t sprite;
  uint8_t frame_idx;
  uint8_t finished;
  const uint8_t *data;
} splash_sprite_t;


void splash_sprite_init(splash_sprite_t *splash, const uint8_t *data);
void splash_sprite_add_to_extents(splash_sprite_t *splash, update_extents_t extents);
void splash_sprite_advance(splash_sprite_t *splash);
bool splash_sprite_is_finished(splash_sprite_t *splash);


#endif /* SPLASH_SPRITE_H */
