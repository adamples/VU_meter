#ifndef UTILS_H
#define UTILS_H

#define int_min(a, b) (((a) < (b)) ? (a) : (b))
#define int_max(a, b) (((a) > (b)) ? (a) : (b))

#define MEMORY_BARRIER() do { __asm volatile("": : :"memory"); } while (0)

#define EEFIXED __attribute__ ((section (".eefixed")))

#endif
