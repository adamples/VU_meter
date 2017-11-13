#ifndef ASSERT_H
#define ASSERT_H

#include "fault.h"

#ifdef NDEBUG
  #define assert(condition) do { } while (0);
#else
  #define assert(condition) \
    do { \
      if (!(condition)) { \
        fault(FAULT_ASSERTION_FAILED, __LINE__, __FILE__); \
      } \
    } while (0);
#endif

#endif /* ASSERT_H */
