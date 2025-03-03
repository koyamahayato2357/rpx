/**
 * @file include/testing.h
 * @brief Define macros for testing
 */

#pragma once

#ifdef TEST_MODE
 #include "ansiesc.h"
 #include "chore.h"
 #include "errcode.h"
 #include "gene.h"
 #include <stdio.h>
 #include <string.h>

extern int TESTING_H_success;
extern int TESTING_H_count;

 #define TEST_HEADER " ■ " ESCBLU "Testing " ESCLR
 #define ALIGN_COL(name) \
   do { \
     int col = 4 - ((int)strlen(#name) + 6) / 8; \
     for (int i = 0; i < col; i++) putchar('\t'); \
   } while (0)
 #define PRINT_FAILED(cnt) printf("\n └" ESCRED ESBLD "[NG:%d]\n" ESCLR, cnt)
 #define PRINT_SUCCESS     puts("=> " ESCGRN "[OK]" ESCLR)

// zig style testing syntax
 #define test(name) \
   void TESTING_H_tester##name(int *); \
   [[gnu::constructor]] void TESTING_H_testrunner##name() { \
     TESTING_H_count++; \
     printf(TEST_HEADER ESBLD #name ESCLR "..."); \
     int failed = 0; \
     TESTING_H_tester##name(&failed); \
     if (failed) { \
       PRINT_FAILED(failed); \
       return; \
     } \
     ALIGN_COL(name); \
     PRINT_SUCCESS; \
     TESTING_H_success++; \
   } \
   void TESTING_H_tester##name(int *TESTING_H_failed)

 #ifdef TEST_FILTER
  #define test_filter(filter) if (strstr(filter, TEST_FILTER))
 #else
  #define test_filter(filter) if (0)
 #endif

 #define GET_M(_1, _2, _3, _4, _5, NAME, ...) NAME

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

 #define CALL(fn, ...) \
   fn(GET_M(__VA_ARGS__, ARGS_4, ARGS_3, ARGS_2, ARGS_1, ARGS_0))
 #define SIGNATURE(...) \
   struct { \
     GET_M(__VA_ARGS__, MEM_DEF_5, MEM_DEF_4, MEM_DEF_3, MEM_DEF_2, MEM_DEF_1) \
     (__VA_ARGS__) \
   }
 #define EXPAND(...) __VA_ARGS__

// if {a, b, c} is passed as a macro parameter, it becomes "{a", "b", "c}", so
// it must be received as a variable length argument.
 #define test_table(name, fn, signature, ...) \
   [[gnu::constructor]] void TESTING_H_tabletester##name() { \
     TESTING_H_count++; \
     printf(TEST_HEADER ESBLD #name ESCLR "..."); \
     int failed = 0; \
     typedef SIGNATURE signature sig_t; \
     sig_t data[] = __VA_ARGS__; \
     for (size_t i = 0; i < sizeof data / sizeof *data; i++) { \
       sig_t *t = data + i; \
       int *TESTING_H_failed /* for expecteq */ = &failed; \
       expecteq(t->expected, CALL(fn, EXPAND signature)); \
     } \
     if (failed) { \
       PRINT_FAILED(failed); \
       return; \
     } \
     ALIGN_COL(name); \
     PRINT_SUCCESS; \
     TESTING_H_success++; \
   }

// disable main function somewhere
 #define main TESTING_H_dummymain

 #define expect(cond) \
   do { \
     if (cond) break; \
     puts("\n ├┬ Unexpected result at " HERE); \
     printf(" │└─ `" #cond "` " ESCRED ESBLD " [NG]" ESCLR); \
     (*TESTING_H_failed)++; \
   } while (0)

 #define expecteq(expected, actual) \
   do { \
     typeof(actual) const lhs = expected; \
     auto const rhs = actual; \
     if (eq((typeof(rhs))lhs, rhs)) break; \
     puts("\n ├┬ Expected equal at " HERE); \
     printf(" │├─ " ESCGRN "Expected" ESCLR ": "); \
     printany((typeof(rhs))lhs); \
     putchar('\n'); \
     printf(" │└─ " ESCRED "Actual" ESCLR ":   "); \
     printany(rhs); \
     printf(ESCRED ESBLD " [NG]" ESCLR); \
     (*TESTING_H_failed)++; \
   } while (0)

 #define expectneq(unexpected, actual) \
   do { \
     typeof(actual) const lhs = unexpected; \
     auto const rhs = actual; \
     if (!eq((typeof(rhs))lhs, rhs)) break; \
     int __llen = (int)strlen(#unexpected); \
     int __rlen = (int)strlen(#actual); \
     int __lpad = bigger(0, __rlen - __llen); \
     int __rpad = bigger(0, __llen - __rlen); \
     puts("\n ├┬ Unexpected equality at " HERE); \
     printf(" │├─ Left side:  `" #unexpected "` ─"); \
     for (int __i = 0; __i < __lpad; __i++) printf("─"); \
     puts("┐"); \
     printf(" │└─ Right side: `" #actual "` ─"); \
     for (int __i = 0; __i < __rpad; __i++) printf("─"); \
     printf("┴─➤ "); \
     printany(lhs); \
     PRINT_SUCCESS; \
     (*TESTING_H_failed)++; \
   } while (0)

 #define testing_unreachable \
   ({ \
    puts("\n ├┬ " ESCRED "Reached line " HERE ESCLR); \
    printf(" │└─ " ESCRED "[NG]" ESCLR); \
    (*TESTING_H_failed)++; \
    (size_t)0; \
   })
#else
// --gc-sections
 #define test(name) \
   [[maybe_unused]] static void TESTING_H_dum##name(int *TESTING_H_failed)
 #define test_table(...)
 #define test_filter(filter)
 #define expect(cond) \
   do { \
     _ = cond; \
     _ = TESTING_H_failed; \
   } while (0)
 #define expecteq(lhs, rhs) \
   do { \
     _ = lhs; \
     _ = rhs; \
     _ = TESTING_H_failed; \
   } while (0)
 #define expectneq(lhs, rhs) \
   do { \
     _ = lhs; \
     _ = rhs; \
     _ = TESTING_H_failed; \
   } while (0)
 #define testing_unreachable _ = TESTING_H_failed
#endif
