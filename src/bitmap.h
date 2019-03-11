#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdbool.h>


typedef uint8_t *bitmap_t;

bool bitmap_get(bitmap_t bitmap, uint8_t bit);
void bitmap_set(bitmap_t bitmap, uint8_t bit, bool value);
void bitmap_fill(bitmap_t bitmap, uint8_t bits_n, bool value);

#endif
