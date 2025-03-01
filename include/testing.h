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

// zig style testing syntax
 #define test(name) \
   void TESTING_H_tester##name(int *); \
   [[gnu::constructor]] void TESTING_H_testrunner##name() { \
     TESTING_H_count++; \
     int TESTING_H_COL = 3 - (strlen(#name) + 3) / 8; \
     printf(ESCBLU "Testing " ESCLR ESBLD #name ESCLR "..."); \
     fflush(stdout); \
     for (int i = 0; i < TESTING_H_COL; i++) putchar('\t'); \
     printf(ESTHN "=> "); \
     int failed = 0; \
     TESTING_H_tester##name(&failed); \
     if (failed) { \
       printf("\n  └" ESCRED ESBLD "[NG:%d]\n" ESCLR, failed); \
       return; \
     } \
     puts(ESCGRN "[OK]" ESCLR); \
     TESTING_H_success++; \
   } \
   void TESTING_H_tester##name(int *TESTING_H_failed [[maybe_unused]])

 #ifndef TEST_FILTER
  #define test_filter(filter) if (0)
 #else
  // zig style `--test-filter`
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
     int failed = 0; \
     for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++) { \
       sig_t *t = data + i; \
       int *TESTING_H_failed /* for expecteq */ = &failed; \
       typeof(t->expected) r = DO_FN(fn, EXPAND signature); \
       expecteq(t->expected, r); \
     } \
     if (failed) { \
       printf("\n  └" ESCRED ESBLD "[NG:%d]\n" ESCLR, failed); \
       return; \
     } \
     puts(ESCGRN ESBLD "[OK]" ESCLR); \
     TESTING_H_success++; \
   }

// disable main function somewhere
 #define main main_

 #define expect(cond) \
   if (!(cond)) { \
     puts("\n  ├┬ Unexpected result at " HERE); \
     printf("  │└─ `" #cond "` " ESCRED ESBLD " [NG]" ESCLR); \
     (*TESTING_H_failed)++; \
   }

 #define expecteq(lhs, rhs) \
   do { \
     if (eq((typeof(rhs))lhs, rhs)) break; \
     puts("\n  ├┬ Expected equal at " HERE); \
     printf("  │├─ " ESCGRN "Expected" ESCLR ": "); \
     printany((typeof(rhs))lhs); \
     putchar('\n'); \
     printf("  │└─ " ESCRED "Actual" ESCLR ":   "); \
     printany(rhs); \
     printf(ESCRED ESBLD " [NG]" ESCLR); \
     (*TESTING_H_failed)++; \
   } while (0)

 #define expectneq(lhs, rhs) \
   do { \
     if (!eq((typeof(rhs))lhs, rhs)) break; \
     size_t __llen = strlen(#lhs); \
     size_t __rlen = strlen(#rhs); \
     size_t __lpad = __llen > __rlen ? 0 : __rlen - __llen; \
     size_t __rpad = __rlen > __llen ? 0 : __llen - __rlen; \
     puts("\n  ├┬ Unexpected equality at " HERE); \
     printf("  │├─ Left side:  `" #lhs "` ─"); \
     for (size_t __i = 0; __i < __lpad; __i++) printf("─"); \
     printf("┐\n"); \
     printf("  │└─ Right side: `" #rhs "` ─"); \
     for (size_t __i = 0; __i < __rpad; __i++) printf("─"); \
     printf("┴─➤ "); \
     printany(lhs); \
     printf(ESCRED ESBLD " [NG]" ESCLR); \
     (*TESTING_H_failed)++; \
   } while (0)

 #define testing_unreachable \
   ({ \
    puts("\n  ├┬ " ESCRED "Reached line " HERE ESCLR); \
    printf("  │└─ " ESCRED "[NG]" ESCLR); \
    (*TESTING_H_failed)++; \
    (size_t)0; \
   })
#else
// --gc-sections
 #define test(name) \
   [[maybe_unused]] static void TESTING_H_dum##name(int *TESTING_H_failed [[maybe_unused]])
 #define test_table(...)
 #define test_filter(filter)
 #define expect(cond)
 #define expecteq(lhs, rhs)
 #define expectneq(lhs, rhs)
 #define testing_unreachable
#endif
