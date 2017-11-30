#ifndef PROGMEM_IMAGE_SPRITE_H
#define PROGMEM_IMAGE_SPRITE_H

#include <stdint.h>
#include "ssd1306.h"
#include "display.h"


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


#endif /* PROGMEM_IMAGE_SPRITE_H */
