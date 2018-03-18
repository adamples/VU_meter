#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

typedef int32_t time_t;


time_t benchmark_start(void);
time_t get_current_time(void);
void benchmark_end(char *name, time_t start_time);


#define BENCHMARK(name, ...) \
  do { \
    time_t start = benchmark_start(); \
    { __VA_ARGS__; } \
    benchmark_end("" # name, start); \
  } while (0)

#endif /* BENCHMARK_H */
