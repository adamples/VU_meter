#ifndef BENCHMARK_H
#define BENCHMARK_H


typedef float time_t;


time_t benchmark_start(void);
void benchmark_end(char *name, time_t start_time);


#define BENCHMARK(name, ...) \
  do { \
    time_t start = benchmark_start(); \
    { __VA_ARGS__; } \
    benchmark_end("" # name, start); \
  } while (0)

#endif /* BENCHMARK_H */
