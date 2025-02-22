#pragma once
#ifdef BENCHMARK_MODE
#include <stdio.h>
#include <time.h>

#ifndef REPEAT
#define REPEAT 10000
#endif

#define bench(name)                                                            \
  void BENCHMARKING_H_bench##name();                                           \
  [[gnu::constructor]] void BENCHMARKING_H_benchrunner##name() {               \
    puts("Benchmarking " #name "...");                                         \
    double duration = 0;                                                       \
    for (int i = 0; i < REPEAT; i++) {                                         \
      clock_t begin = clock();                                                 \
      [[clang::always_inline]] BENCHMARKING_H_bench##name();                   \
      clock_t end = clock();                                                   \
      duration += difftime(end, begin) / CLOCKS_PER_SEC * 1e6;                 \
    }                                                                          \
    printf("Excution time: %.6f microsecs\n", duration / REPEAT);              \
  }                                                                            \
  void BENCHMARKING_H_bench##name()
#define main main_
#else
#define bench(a) void BENCHMARKING_H_dummy##a()
#endif
