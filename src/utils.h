#ifndef UTILS_H
#define UTILS_H

#include <avr/wdt.h>

#define int_min(a, b) (((a) < (b)) ? (a) : (b))
#define int_max(a, b) (((a) > (b)) ? (a) : (b))

#define MEMORY_BARRIER() do { __asm volatile("": : :"memory"); } while (0)
#define EEFIXED __attribute__ ((section (".eefixed")))


void delay_ms(uint16_t t);


void watchdog_init(void);

#ifdef NDEBUG
  #define watchdog_reset() wdt_reset()
#else
  #define watchdog_reset() do { } while (0)
#endif


#endif
