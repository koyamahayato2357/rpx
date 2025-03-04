/**
 * @file include/benchmarking.h
 * @brief Define macros for benchmarking
 */

#pragma once
#ifdef BENCHMARK_MODE
 #include "ansiesc.h"
 #include <stdio.h>
 #include <time.h>

 #ifndef REPEAT
  #define REPEAT 10'000
 #endif

 #define BENCH_HEADER " ■ " ESCBLU "Benchmarking " ESCLR

 #define bench(name) \
   void BENCH_bench##name(); \
   [[gnu::constructor]] void BENCH_run##name() { \
     printf(BENCH_HEADER ESBLD #name ESCLR "..."); \
     double duration = 0; \
     for (int i = 0; i < REPEAT; i++) { \
       clock_t begin = clock(); \
       [[clang::always_inline]] BENCH_bench##name(); \
       clock_t end = clock(); \
       duration += difftime(end, begin) / CLOCKS_PER_SEC * 1e6; \
     } \
     printf(" => %.6f microsecs\n", duration / REPEAT); \
   } \
   void BENCH_bench##name()

 #define bench_cycle(name) \
   void BENCH_benchcycle##name(); \
   [[gnu::constructor]] void BENCH_runcycle##name() { \
     printf(BENCH_HEADER ESBLD #name ESCLR "..."); \
     double cycle = 0; \
     for (int i = 0; i < REPEAT; i++) { \
       unsigned long long begin = __builtin_readcyclecounter(); \
       [[clang::always_inline]] BENCH_benchcycle##name(); \
       unsigned long long end = __builtin_readcyclecounter(); \
       cycle += (double)(end - begin); \
     } \
     printf(" => %.6f cycle\n", cycle / REPEAT); \
   } \
   void BENCH_benchcycle##name()

 #define main BENCH_dummymain
#else
// --gc-sections
 #define bench(a)       [[gnu::unused]] static void BENCH_dum##a()
 #define bench_cycle(a) [[gnu::unused]] static void BENCH_dumc##a()
#endif
