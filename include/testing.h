/**
 * @file include/testing.h
 * @brief Define macros for testing
 * @note Tests not in the test_filter block are always executed
 */

#pragma once

#ifdef TEST_MODE
 #include "ansiesc.h"
 #include "chore.h"
 #include "errcode.h"
 #include "gene.h"
 #include <stdio.h>
 #include <string.h>

extern int TEST_success;
extern int TEST_count;

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
   void TEST_test##name(int *); \
   [[gnu::constructor]] void TEST_run##name() { \
     TEST_count++; \
     printf(TEST_HEADER ESBLD #name ESCLR "..."); \
     int failed = 0; \
     TEST_test##name(&failed); \
     if (failed) { \
       PRINT_FAILED(failed); \
       return; \
     } \
     ALIGN_COL(name); \
     PRINT_SUCCESS; \
     TEST_success++; \
   } \
   void TEST_test##name(int *TEST_failed)

 #ifdef TEST_FILTER
  #define test_filter(filter) if (strstr(filter, TEST_FILTER))
 #else
  #define test_filter(filter) if (0)
 #endif

 #define GETM(_1, _2, _3, _4, _5, NAME, ...) NAME
// function parameter
 #define PARAM0
 #define PARAM1 t->p1
 #define PARAM2 PARAM1, t->p2
 #define PARAM3 PARAM2, t->p3
 #define PARAM4 PARAM3, t->p4
// define struct member
 #define STMEM1(_1)                 _1 expected;
 #define STMEM2(_1, _2)             STMEM1(_1) _2 p1;
 #define STMEM3(_1, _2, _3)         STMEM2(_1, _2) _3 p2;
 #define STMEM4(_1, _2, _3, _4)     STMEM3(_1, _2, _3) _4 p3;
 #define STMEM5(_1, _2, _3, _4, _5) STMEM4(_1, _2, _3, _4) _5 p4;

 #define CALL(fn, ...) \
   fn(GETM(__VA_ARGS__, PARAM4, PARAM3, PARAM2, PARAM1, PARAM0))
 #define STDEF(...) \
   struct { \
     GETM(__VA_ARGS__, STMEM5, STMEM4, STMEM3, STMEM2, STMEM1)(__VA_ARGS__) \
   }
 #define EXPAND(...) __VA_ARGS__

// if {a, b, c} is passed as a macro parameter, it becomes "{a", "b", "c}", so
// it must be received as a variable length argument.
 #define test_table(name, fn, signature, ...) \
   [[gnu::constructor]] void TEST_tabletest##name() { \
     TEST_count++; \
     printf(TEST_HEADER ESBLD #name ESCLR "..."); \
     int failed = 0; \
     typedef STDEF signature S; \
     S data[] = __VA_ARGS__; \
     for (size_t i = 0; i < sizeof data / sizeof(S); i++) { \
       S *t = data + i; \
       int *TEST_failed /* for expecteq */ = &failed; \
       expecteq(t->expected, CALL(fn, EXPAND signature)); \
     } \
     if (failed) { \
       PRINT_FAILED(failed); \
       return; \
     } \
     ALIGN_COL(name); \
     PRINT_SUCCESS; \
     TEST_success++; \
   }

// disable main function somewhere
 #define main TEST_dummymain

 #define expect(cond) \
   do { \
     if (cond) break; \
     puts("\n ├┬ Unexpected result at " HERE); \
     printf(" │└─ `" #cond "` " ESCRED ESBLD " [NG]" ESCLR); \
     (*TEST_failed)++; \
   } while (0)

 #define expecteq(expected, actual) \
   do { \
     typeof(actual) const lhs = expected; \
     auto const rhs = actual; \
     if (eq(lhs, rhs)) break; \
     puts("\n ├┬ Expected equal at " HERE); \
     printf(" │├─ " ESCGRN "Expected" ESCLR ": "); \
     printany(lhs); \
     putchar('\n'); \
     printf(" │└─ " ESCRED "Actual" ESCLR ":   "); \
     printany(rhs); \
     printf(ESCRED ESBLD " [NG]" ESCLR); \
     (*TEST_failed)++; \
   } while (0)

 #define expectneq(unexpected, actual) \
   do { \
     typeof(actual) const lhs = unexpected; \
     auto const rhs = actual; \
     if (!eq(lhs, rhs)) break; \
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
     printf(ESCRED ESBLD " [NG]" ESCLR); \
     (*TEST_failed)++; \
   } while (0)

 #define testing_unreachable \
   ({ \
    puts("\n ├┬ " ESCRED "Reached line " HERE ESCLR); \
    printf(" │└─ " ESCRED ESBLD "[NG]" ESCLR); \
    (*TEST_failed)++; \
    (size_t)0; \
   })
#else
// --gc-sections
 #define test(name) [[gnu::unused]] static void TEST_dum##name(int *TEST_failed)
 #define test_table(...)
 #define test_filter(filter)
 #define expect(cond) \
   do { \
     _ = cond; \
     _ = TEST_failed; \
   } while (0)
 #define expecteq(lhs, rhs) \
   do { \
     _ = lhs; \
     _ = rhs; \
     _ = TEST_failed; \
   } while (0)
 #define expectneq(lhs, rhs) \
   do { \
     _ = lhs; \
     _ = rhs; \
     _ = TEST_failed; \
   } while (0)
 #define testing_unreachable _ = TEST_failed
#endif
