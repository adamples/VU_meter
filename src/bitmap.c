#include "bitmap.h"
#include <assert.h>
#include <stdlib.h>

#define BITMAP_INT_BIT_SIZE (8)


bool
bitmap_get(bitmap_t bitmap, uint8_t bit)
{
  uint8_t word_idx = bit / BITMAP_INT_BIT_SIZE;
  uint8_t bit_idx = bit % BITMAP_INT_BIT_SIZE;
  uint8_t word = bitmap[word_idx];

  return word & (1 << bit_idx);
}


void
bitmap_set(bitmap_t bitmap, uint8_t bit, bool value)
{
  uint8_t word_idx = bit / BITMAP_INT_BIT_SIZE;
  uint8_t bit_idx = bit % BITMAP_INT_BIT_SIZE;

  if (value) {
    bitmap[word_idx] |= (1 << bit_idx);
  }
  else {
    bitmap[word_idx] &= ~(1 << bit_idx);
  }
}


void
bitmap_fill(bitmap_t bitmap, uint8_t bits_n, bool value)
{
  assert(bitmap);

  uint8_t word_value = (value) ? 0xff : 0x00;

  for (uint8_t i = 0; i < bits_n / BITMAP_INT_BIT_SIZE; ++i) {
    bitmap[i] = word_value;
  }
}
