#pragma once
#include <setjmp.h>

#ifdef TEST_MODE
 #include "ansiesc.h"
 #include "errcode.h"
 #include "gene.h"
 #include <stdio.h>
 #include <string.h>

extern int TESTING_H_success;
extern int TESTING_H_count;

// zig style testing syntax
 #define test(name) \
   void TESTING_H_tester##name(jmp_buf); \
   [[gnu::constructor]] void TESTING_H_testrunner##name() { \
     TESTING_H_count++; \
     int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8; \
     jmp_buf jb; \
     printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "..."); \
     fflush(stdout); \
     for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++) \
       putchar('\t'); \
     printf(ESTHN "=> "); \
     if (setjmp(jb) == 0) TESTING_H_tester##name(jb); \
     else { \
       puts(ESCRED "[NG]" ESCLR); \
       return; \
     } \
     puts(ESCGRN "[OK]" ESCLR); \
     TESTING_H_success++; \
   } \
   void TESTING_H_tester##name(jmp_buf jb [[maybe_unused]])

 #ifndef TEST_FILTER
  #define test_filter(filter) if (0)
 // zig style `--test-filter`
 #else
  #define test_filter(filter) if (strstr(filter, TEST_FILTER))
 #endif

 #define ARGS_0
 #define ARGS_1 t->a1
 #define ARGS_2 ARGS_1, t->a2
 #define ARGS_3 ARGS_2, t->a3
 #define ARGS_4 ARGS_3, t->a4

 #define MEM_DEF_1(_1)                 _1 expected;
 #define MEM_DEF_2(_1, _2)             MEM_DEF_1(_1) _2 a1;
 #define MEM_DEF_3(_1, _2, _3)         MEM_DEF_2(_1, _2) _3 a2;
 #define MEM_DEF_4(_1, _2, _3, _4)     MEM_DEF_3(_1, _2, _3) _4 a3;
 #define MEM_DEF_5(_1, _2, _3, _4, _5) MEM_DEF_4(_1, _2, _3, _4) _5 a4;

 #define EXPAND(...) __VA_ARGS__

 #define GET_M(_1, _2, _3, _4, _5, NAME, ...) NAME

 #define DO_FN(fn, ...) \
   fn(GET_M(__VA_ARGS__, ARGS_4, ARGS_3, ARGS_2, ARGS_1, ARGS_0))

 #define SIGNATURE(...) \
   struct { \
     GET_M(__VA_ARGS__, MEM_DEF_5, MEM_DEF_4, MEM_DEF_3, MEM_DEF_2, MEM_DEF_1) \
     (__VA_ARGS__) \
   }

// if {a, b, c} is passed as a macro parameter, it becomes "{a", "b", "c}", so
// it must be received as a variable length argument.
// the max num of fn params is 4, but thats enough, right?
 #define test_table(name, fn, signature, ...) \
   [[gnu::constructor]] void TESTING_H_tabletester##name() { \
     TESTING_H_count++; \
     printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "..."); \
     int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8; \
     for (int TESTING_H_i = 0; TESTING_H_i < TESTING_H_COL; TESTING_H_i++) \
       putchar('\t'); \
     printf(ESTHN "=> "); \
     fflush(stdout); \
     typedef SIGNATURE signature sig_t; \
     sig_t data[] = __VA_ARGS__; \
     for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++) { \
       sig_t *t = data + i; \
       typeof(t->expected) r = DO_FN(fn, EXPAND signature); \
       if (eq(r, t->expected)) continue; \
       printf("Test case %zu failed: expected ", i); \
       printany(t->expected); \
       printf(" found "); \
       printany(r); \
       puts(" " ESCRED "[NG]" ESCLR); \
       return; \
     } \
     puts(ESCGRN "[OK]" ESCLR); \
     TESTING_H_success++; \
   }

// disable main function somewhere
 #define main main_

 #define expect(cond) \
   if (!(cond)) { \
     puts("Failed at " HERE " " #cond " "); \
     longjmp(jb, 1); \
   }

 #define expecteq(lhs, rhs) \
   do { \
     if (eq((typeof(rhs))lhs, rhs)) break; \
     printf("Expected "); \
     printany(lhs); \
     printf(" found "); \
     printany(rhs); \
     printf(" at " HERE); \
     longjmp(jb, 1); \
   } while (0)

 #define expectneq(lhs, rhs) \
   do { \
     if (!eq((typeof(rhs))lhs, rhs)) break; \
     printf("Not expected equal "); \
     printf(#lhs); \
     printf(" and "); \
     printf(#rhs); \
     printf(" at " HERE); \
     longjmp(jb, 1); \
   } while (0)

 #define testing_unreachable \
   ({ \
    puts(ESCRED "Reached line " HERE ESCLR); \
    longjmp(jb, ERR_REACHED_UNREACHABLE); \
    (size_t)0; \
   })
#else
// --gc-sections
 #define test(name) \
   [[maybe_unused]] static void TESTING_H_dum##name(jmp_buf jb [[maybe_unused]])
 #define test_table(...)
 #define test_filter(filter)
 #define expect(cond)
 #define expecteq(lhs, rhs)
 #define expectneq(lhs, rhs)
 #define testing_unreachable
#endif
