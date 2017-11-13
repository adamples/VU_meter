#include "drawing.h"
#include <stdlib.h>


void draw_line_23_octants(int8_t buffer[], uint8_t ax, uint8_t ay, uint8_t bx, uint8_t by)
{
  int8_t error = 0;
  int8_t delta_err = abs(bx - ax);
  int8_t error_threshold = (by - ay) / 2;
  int8_t delta_x = (bx > ax) ? 1 : -1;

  for (int8_t y = ay, x = ax; y <= by; ++y) {
    buffer[y] = x;
    error += delta_err;

    if (error >= error_threshold) {
      x += delta_x;
      error -= delta_err;
    }
  }
}
