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
#define ARGS_0
#define ARGS_1 t->a1
#define ARGS_2 ARGS_1, t->a2
#define ARGS_3 ARGS_2, t->a3
#define ARGS_4 ARGS_3, t->a4
#define ARGS_N(n) ARGS_##n
#define GET_ARG_1(_1, ...) _1
#define GET_ARG_2(_1, _2, ...) _2
#define GET_ARG_3(_1, _2, _3, ...) _3
#define GET_ARG_4(_1, _2, _3, _4, ...) _4
#define GET_ARG_5(_1, _2, _3, _4, _5, ...) _5
#define GET_ARG_N(n, ...) GET_ARG_##n(__VA_ARGS__)
#define VA_ARGC(...) _VA_ARGC(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)
#define _VA_ARGC(_1, _2, _3, _4, _5, _6, _7, c, ...) c
#define MEM_DEF_1(_1) _1 a0;
#define MEM_DEF_2(_1, _2) MEM_DEF_1(_1) _2 a1;
#define MEM_DEF_3(_1, _2, _3) MEM_DEF_2(_1, _2) _3 a2;
#define MEM_DEF_4(_1, _2, _3, _4) MEM_DEF_3(_1, _2, _3) _4 a3;
#define MEM_DEF_5(_1, _2, _3, _4, _5) MEM_DEF_4(_1, _2, _3, _4) _5 a4;
#define GET_M(_1, _2, _3, _4, _5, NAME, ...) NAME
#define EXPAND(...) __VA_ARGS__
#define DO_FN(fn, ...)                                                         \
  fn(GET_M(__VA_ARGS__, ARGS_4, ARGS_3, ARGS_2, ARGS_1, ARGS_0))
#define SIGNATURE(...)                                                         \
  struct {                                                                     \
    GET_M(__VA_ARGS__, MEM_DEF_5, MEM_DEF_4, MEM_DEF_3, MEM_DEF_2, MEM_DEF_1)  \
    (__VA_ARGS__)                                                              \
  }
#define test_table(name, fn, signature, ...)                                   \
  typedef SIGNATURE signature ds##name;                                        \
  __attribute__((constructor)) void TESTING_H_tabletester##name() {            \
    ds##name data[] = __VA_ARGS__;                                             \
    int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8;                           \
    printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "...");                   \
    fflush(stdout);                                                            \
    for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++)      \
      putchar('\t');                                                           \
    printf(ESTHN "=> ");                                                       \
    for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++) {              \
      ds##name *t = data + i;                                                  \
      typeof(t->a0) result = DO_FN(fn, EXPAND signature);                      \
      if (!eq(result, t->a0)) {                                                \
        printf("Test case %zu failed: expected ", i);                          \
        printany(t->a0);                                                       \
        printf(" found ");                                                     \
        printany(result);                                                      \
        puts(" " ESCRED "[NG]" ESCLR);                                         \
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
