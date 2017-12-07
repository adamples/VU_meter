#ifndef _FIX_POINT_H_
#define _FIX_POINT_H_

/* - User settings ---------------------------------------------------------- */

#define FIX_INT_DIGITS (8)
#define FIX_FRAC_DIGITS (8)
#define FIX_HIGH_PRECISION (1)

/* - end of User settings --------------------------------------------------- */

#include <stdint.h>
#include <math.h>

#define FIX_DIGITS (FIX_INT_DIGITS + FIX_FRAC_DIGITS)
#define FIX_ONE ((fix_t) 1 << FIX_FRAC_DIGITS)

#define DFIX_INT_DIGITS (FIX_INT_DIGITS * 2)
#define DFIX_FRAC_DIGITS (FIX_FRAC_DIGITS * 2)
#define DFIX_DIGITS (DFIX_INT_DIGITS + DFIX_FRAC_DIGITS)
#define DFIX_ONE ((dfix_t) 1 << DFIX_FRAC_DIGITS)


#if (FIX_DIGITS == 8)
  typedef int8_t fix_t;
  typedef uint8_t ufix_t;
  typedef int16_t dfix_t;
  typedef uint16_t udfix_t;
  typedef int32_t ddfix_t;
#elif (FIX_DIGITS == 16)
  typedef int16_t fix_t;
  typedef uint16_t ufix_t;
  typedef int32_t dfix_t;
  typedef uint32_t udfix_t;
  typedef int64_t ddfix_t;
#elif (FIX_DIGITS == 32)
  typedef int32_t fix_t;
  typedef uint32_t ufix_t;
  typedef int64_t dfix_t;
  typedef uint64_t udfix_t;
  typedef int64_t ddfix_t;
#else
  #error Unsupported number of digits for fixed point type (must be 8, 16 or 32)
#endif


#if FIX_HIGH_PRECISION
#define FIX_DIV_BY_INT(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))
#else
#define FIX_DIV_BY_INT(n, d) ((n) / (d))
#endif


#define FIX_MIN ((fix_t) 1 << (FIX_DIGITS - 1))
#define FIX_MAX ((fix_t) ~FIX_MIN)

#define DFIX_MIN ((dfix_t) 1 << (DFIX_DIGITS - 1))
#define DFIX_MAX ((dfix_t) ~DFIX_MIN)


#define FIX_GET_INT(a) ((a) / FIX_ONE)
#define FIX_GET_FRAC(a) ((ufix_t) FIX_ABS(a) & (FIX_ONE - 1))

#define DFIX_GET_INT(a) ((a) / DFIX_ONE)
#define DFIX_GET_FRAC(a) ((udfix_t) FIX_ABS(a) & (DFIX_ONE - 1))

#define FIX_ABS(a) (((a) >= 0) ? (a) : -(a))

#define FIX_NEG(a) (-(a))

#define FIX_ADD(a, b) ((a) + (b))
#define FIX_ADD_INPL(a, b) do { a += b; } while (0)

#define FIX_SUB(a, b) ((a) - (b))
#define FIX_SUB_INPL(a, b) do { a -= b; } while (0)

#define FIX_DMUL(a, b) ((dfix_t) (a) * (b))
#define FIX_D2S(a) FIX_DIV_BY_INT(a, FIX_ONE)
#define FIX_MUL(a, b) (FIX_D2S(FIX_DMUL(a, b)))

#define FIX_DIV(a, b) FIX_DIV_BY_INT((dfix_t) (a) * FIX_ONE, b)

#define FIX_DSQR(a) (FIX_DMUL(a, a))
#define FIX_SQR(a) (FIX_MUL(a, a))

#define FIX_TO_FLOAT(a) ((float) (a) / FIX_ONE)
#define DFIX_TO_FLOAT(a) ((float) (a) / DFIX_ONE)
#define FIX_FROM_FLOAT(a) ((fix_t) lround((float) (a) * FIX_ONE))
#define DFIX_FROM_FLOAT(a) ((dfix_t) lround((float) (a) * DFIX_ONE))

#define FIX_TO_INT(a) FIX_DIV_BY_INT(a, FIX_ONE)
#define DFIX_TO_INT(a) FIX_DIV_BY_INT(a, DFIX_ONE)
#define FIX_FROM_INT(a) ((fix_t) (a) * FIX_ONE)
#define DFIX_FROM_INT(a) ((dfix_t) (a) * DFIX_ONE)

#define FIX_FRACTION(n, d) FIX_DIV(n, d)

/*
 * after: Turkowski, K., Fixed Point Square Root, Apple Technical Report No. 96, 1994.
 */
udfix_t dfix_sqrt(udfix_t a);
ufix_t fix_sqrt(ufix_t a);

/*
 * after: Turner, Clay S., A Fast Binary Logarithm Algorithm, IEEE SIGNAL PROCESSING MAGAZINE, Volume: 27, Issue: 5, Sept. 2010.
 * http://ieeexplore.ieee.org/document/5562652/
 * http://www.claysturner.com/dsp/BinaryLogarithm.pdf
 */
fix_t fix_log2(ufix_t x);
fix_t dfix_log2(udfix_t x);

#endif
