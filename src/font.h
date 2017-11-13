#ifndef FONT_H
#define FONT_H

#include <stdint.h>


typedef struct font_header_t_ {
  uint8_t height;
  uint8_t char_ranges_n;
} font_header_t;

typedef struct char_range_header_t_ {
  uint8_t size;
  uint8_t first_character;
} char_range_header_t;


typedef struct my_font_t_ {
  font_header_t header;
  char_range_header_t digits;
  uint8_t digit_widths[5];
  uint8_t digit_glyphs;
} my_font;


#endif /* FONT_H */
