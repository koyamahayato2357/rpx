#pragma once
#include "ansiesc.h"
#include "gene.h"
#include <setjmp.h>
#include <stdio.h>

#ifdef TEST_MODE
#include "errcode.h"
#include <string.h>

extern int TESTING_H_success;
extern int TESTING_H_fail;
#define test(name)                                                             \
  void TESTING_H_tester##name(jmp_buf);                                        \
  __attribute__((constructor)) void TESTING_H_testrunner##name() {             \
    int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8;                           \
    jmp_buf jb;                                                                \
    printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "...");                   \
    fflush(stdout);                                                            \
    for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++)      \
      putchar('\t');                                                           \
    printf(ESTHN "=> ");                                                       \
    if (setjmp(jb) == 0)                                                       \
      TESTING_H_tester##name(jb);                                              \
    else {                                                                     \
      puts(ESCRED "[NG]" ESCLR);                                               \
      TESTING_H_fail++;                                                        \
      return;                                                                  \
    }                                                                          \
    puts(ESCGRN "[OK]" ESCLR);                                                 \
    TESTING_H_success++;                                                       \
  }                                                                            \
  void TESTING_H_tester##name(jmp_buf jb [[maybe_unused]])
#define EXPAND(x) x
#define ARGS_0
#define ARGS_1 t->a1
#define ARGS_2 t->a1, t->a2
#define ARGS_3 t->a1, t->a2, t->a3
#define ARGS_4 t->a1, t->a2, t->a3, t->a4
#define ARGS_5 t->a1, t->a2, t->a3, t->a4, t->a5
#define ARGS_6 t->a1, t->a2, t->a3, t->a4, t->a5, t->a6
#define ARGS_7 t->a1, t->a2, t->a3, t->a4, t->a5, t->a6, t->a7
#define ARGS_N(n) ARGS_##n
#define test_table(name, fn, fargc, ...)                                       \
  _Static_assert(fargc <= 7, "too many argument");                             \
  typedef typeof(__VA_ARGS__[0]) ds##name;                                     \
  __attribute__((constructor)) void TESTING_H_tabletester##name() {            \
    ds##name *data = (ds##name *)__VA_ARGS__;                                  \
    int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8;                           \
    printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "...");                   \
    fflush(stdout);                                                            \
    for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++)      \
      putchar('\t');                                                           \
    printf(ESTHN "=> ");                                                       \
    for (size_t i = 0; i < sizeof(__VA_ARGS__) / sizeof(__VA_ARGS__[0]);       \
         i++) {                                                                \
      ds##name *t = &data[i];                                                  \
      auto result = fn(ARGS_N(fargc));                                         \
      if (result != t->result) {                                               \
        printf("Record %zu expected ", i);                                     \
        printany(t->result);                                                   \
        printf(" found ");                                                     \
        printany(result);                                                      \
        putchar(' ');                                                          \
        puts(ESCRED "[NG]" ESCLR);                                             \
        TESTING_H_fail++;                                                      \
        return;                                                                \
      }                                                                        \
    }                                                                          \
    puts(ESCGRN "[OK]" ESCLR);                                                 \
    TESTING_H_success++;                                                       \
  }

#define main main_
#else
#define test(name) void TESTING_H_dummy##name(jmp_buf jb [[maybe_unused]])
#define test_table(...)
#endif

#define expect(cond)                                                           \
  if (!(cond)) {                                                               \
    puts("Failed at " HERE " " #cond " ");                                     \
    longjmp(jb, 1);                                                            \
  }

#define expecteq(lhs, rhs)                                                     \
  do {                                                                         \
    if (eq((typeof(rhs))lhs, rhs))                                             \
      break;                                                                   \
    printf("Expected ");                                                       \
    printany(lhs);                                                             \
    printf(" found ");                                                         \
    printany(rhs);                                                             \
    printf(" at " HERE);                                                       \
    longjmp(jb, 1);                                                            \
  } while (0)

#define expectneq(lhs, rhs)                                                    \
  do {                                                                         \
    if (!eq((typeof(rhs))lhs, rhs))                                            \
      break;                                                                   \
    printf("Not expected equal ");                                             \
    printf(#lhs);                                                              \
    printf(" and ");                                                           \
    printf(#rhs);                                                              \
    printf(" at " HERE);                                                       \
    longjmp(jb, 1);                                                            \
  } while (0)

#define testing_unreachable                                                    \
  ({                                                                           \
    puts(ESCRED "Reached line " HERE ESCLR);                                   \
    longjmp(jb, ERR_REACHED_UNREACHABLE);                                      \
    (size_t)0;                                                                 \
  })
