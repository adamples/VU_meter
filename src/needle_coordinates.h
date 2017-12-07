#ifndef NEEDLE_COORDINATES_H
#define NEEDLE_COORDINATES_H

#include <stdint.h>
#include "config.h"


#define NEEDLE_AXIS_X (64)
#define NEEDLE_AXIS_Y (96)

typedef struct needle_coordinates_t_ {
  uint8_t x;
  uint8_t y;
  /* by is implicit = SSD1306_HEIGHT - 1 */
} needle_coordinates_t;


extern const needle_coordinates_t NEEDLE_COORDINATES[NEEDLE_RESOLUTION];

#endif /* NEEDLE_COORDINATES_H */
