#ifndef NEEDLE_SPRITE_H
#define NEEDLE_SPRITE_H

#include <stdint.h>
#include "ssd1306.h"
#include "display.h"


typedef struct needle_sprite_t_ {
  sprite_t sprite;
  int8_t column[SSD1306_HEIGHT];
} needle_sprite_t;


void needle_sprite_init(needle_sprite_t *needle);
void needle_sprite_draw(needle_sprite_t *needle, uint8_t angle);

void draw_line_23_octants(int8_t buffer[], uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by);


#endif /* NEEDLE_SPRITE_H */
