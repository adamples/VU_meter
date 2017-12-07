#include "fixed_point.h"


/* - Square root ------------------------------------------------------------ */

#define DSQRT_CNT_MAX (DFIX_DIGITS - FIX_INT_DIGITS)
#define DSQRT_FIX_SIZE_SUB2 (DFIX_DIGITS - 2)

udfix_t
dfix_sqrt(udfix_t a)
{
  udfix_t root = 0;
  udfix_t rem_hi = 0;
  udfix_t rem_lo = a;
  udfix_t test_div;

  for (uint8_t i = DSQRT_CNT_MAX; i > 0; --i) {
    rem_hi = (rem_hi << 2) | (rem_lo >> DSQRT_FIX_SIZE_SUB2);
    rem_lo <<= 2;
    root <<= 1;
    test_div = (root << 1) + 1;

    if (rem_hi >= test_div) {
      rem_hi -= test_div;
      ++root;
    }
  }

  return root;
}

#define SQRT_CNT_MAX (FIX_DIGITS - FIX_INT_DIGITS / 2)
#define SQRT_FIX_SIZE_SUB2 (FIX_DIGITS - 2)

ufix_t
fix_sqrt(ufix_t a)
{
  ufix_t root = 0;
  ufix_t rem_hi = 0;
  ufix_t rem_lo = a;
  ufix_t test_div;

  for (uint8_t i = SQRT_CNT_MAX; i > 0; --i) {
    rem_hi = (rem_hi << 2) | (rem_lo >> SQRT_FIX_SIZE_SUB2);
    rem_lo <<= 2;
    root <<= 1;
    test_div = (root << 1) + 1;

    if (rem_hi >= test_div) {
      rem_hi -= test_div;
      ++root;
    }
  }

  return root;
}

/* - end of Square root ----------------------------------------------------- */

/* - Logarithms ------------------------------------------------------------- */

#define FIX_TWO (FIX_ONE << 1)
#define DFIX_TWO (DFIX_ONE << 1)

fix_t
fix_log2(ufix_t x)
{
  if (x == 0) return FIX_MIN;

  fix_t y = 0;

  while (x < FIX_ONE) {
    x <<= 1;
    y -= FIX_ONE;
  }

  while (x >= FIX_TWO) {
    x = FIX_DIV_BY_INT(x, 2);
    y += FIX_ONE;
  }

  for (ufix_t b = FIX_ONE >> 1; x != 0 && b != 0; b >>= 1) {
    x = FIX_MUL(x, x);
    if (x >= FIX_TWO) {
      x = FIX_DIV_BY_INT(x, 2);
      y |= b;
    }
  }

  return y;
}


fix_t
dfix_log2(udfix_t x)
{
  /* Plase note that in this function x is double precision number, while y is
   * still single precision one. Because of this x is compared against DFIX_*
   * macros and y and b are modified using FIX_*.
   */
  if (x == 0) return FIX_MIN;

  fix_t y = 0;

  while (x < DFIX_ONE) {
    x <<= 1;
    y -= FIX_ONE;
  }

  while (x >= DFIX_TWO) {
    x = FIX_DIV_BY_INT(x, 2);
    y += FIX_ONE;
  }

  for (ufix_t b = FIX_ONE >> 1; x != 0 && b != 0; b >>= 1) {
    x = FIX_DIV_BY_INT((ddfix_t) x * x, DFIX_ONE);
    if (x >= DFIX_TWO) {
      x = FIX_DIV_BY_INT(x, 2);
      y |= b;
    }
  }

  return y;
}

/* - end of Logarithms ------------------------------------------------------ */
