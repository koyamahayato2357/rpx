#pragma once
#include "exception.h"
#include <stdio.h>
#include <string.h>
#include <tgmath.h>

#ifdef TEST_MODE
extern int TESTING_H_success;
extern int TESTING_H_fail;
#define test(name)                                                             \
  void TESTING_H_tester##name();                                               \
  __attribute__((constructor)) void TESTING_H_testrunner##name() {             \
    int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8;                           \
    printf("\033[34mTesting\033[0m " #name "...");                             \
    for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++)      \
      putchar('\t');                                                           \
    printf("\033[2m=> ");                                                      \
    try TESTING_H_tester##name();                                              \
    catchany capture(e) {                                                      \
      puts("\033[31m[NG]\033[0m");                                             \
      TESTING_H_fail++;                                                        \
      return;                                                                  \
    }                                                                          \
    puts("\033[32m[OK]\033[0m");                                               \
    TESTING_H_success++;                                                       \
  }                                                                            \
  void TESTING_H_tester##name()

#define main main_
#else
#define test(name) void TESTING_H_dummy##name()
#endif

#define expect(cond)                                                           \
  if (!(cond)) {                                                               \
    printf("Failed at %s:%d %s ", __FILE__, __LINE__, #cond);                   \
    throw(1);                                                                  \
  }

#define EPSILON 1e-5
// Helper function to compare doubles with epsilon
bool double_eq(double, double);

bool complex_eq(double complex, double complex);

#define _unreachable(line)                                                     \
  do {                                                                         \
    printf("\033[31mReached line %d\033[0m\n", line);                          \
    throw(1);                                                                  \
  } while (0)
#define unreachable _unreachable(__LINE__)
