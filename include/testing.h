#pragma once
#include "ansiesc.h"
#include "errcode.h"
#include "gene.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#ifdef TEST_MODE
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
#define GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
#define FOR_EACH_1(macro, a) macro(a)
#define FOR_EACH_2(macro, a, ...)                                              \
  macro(a), EXPAND(FOR_EACH_1(macro, __VA_ARGS__))
#define FOR_EACH_3(macro, a, ...)                                              \
  macro(a), EXPAND(FOR_EACH_2(macro, __VA_ARGS__))
#define FOR_EACH_4(macro, a, ...)                                              \
  macro(a), EXPAND(FOR_EACH_3(macro, __VA_ARGS__))
#define FOR_EACH(...)                                                          \
  GET_MACRO(__VA_ARGS__, FOR_EACH_4, FOR_EACH_3, FOR_EACH_2, FOR_EACH_1)       \
  (__VA_ARGS__)
#define MEMBER_NAME(member) t->member
#define MEMBER_NAMES(...) FOR_EACH(MEMBER_NAME, __VA_ARGS__)
#define test_table(name, fn, datast, cases, ...)                               \
  typedef datast ds##name;                                                     \
  __attribute__((constructor)) void TESTING_H_tabletester##name() {            \
    ds##name *data = cases;                                                    \
    int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8;                           \
    printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "...");                   \
    fflush(stdout);                                                            \
    for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++)      \
      putchar('\t');                                                           \
    printf(ESTHN "=> ");                                                       \
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {            \
      ds##name *t = &data[i];                                                  \
      auto result = fn(MEMBER_NAMES(__VA_ARGS__));                             \
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

#define testing_unreachable                                                    \
  ({                                                                           \
    puts(ESCRED "Reached line " HERE ESCLR);                                   \
    longjmp(jb, ERR_REACHED_UNREACHABLE);                                      \
    (size_t)0;                                                                 \
  })
