/**
 * @file include/chore.h
 * @brief Define util macros
 */

#pragma once
#include "def.h"
#include <dirent.h>
#include <stdio.h>

constexpr size_t alpha_n = 'z' - 'a' + 1;

#define HERE             __FILE__ ":" TOSTR(__LINE__)
#define lesser(lhs, rhs) ((lhs) < (rhs) ? (lhs) : (rhs))
#define bigger(lhs, rhs) ((lhs) > (rhs) ? (lhs) : (rhs))
#define overloadable     [[clang::overloadable]]
#define ondrop(cl)       [[gnu::cleanup(cl)]]
#define drop             ondrop(freecl)
#define dropfile         ondrop(fclosecl)
#define dropdir          ondrop(closedircl)
#define _                auto CAT(_DISCARD_, __COUNTER__) [[gnu::unused]]

// zig style alloc()
#define zalloc(T, size) ((T *)palloc(size * sizeof(T)))

#define nfree(p) \
  do { \
    if (p == nullptr) break; \
    free(p); \
    p = nullptr; \
  } while (0)

struct winsize getWinSize();
[[gnu::const]] bool isInt(double);
[[gnu::nonnull]] overloadable void skipSpaces(char const **);
[[gnu::nonnull]] overloadable void skipSpaces(char const **, size_t);
[[gnu::nonnull]] void skipUntilComma(char const **);
[[gnu::returns_nonnull, nodiscard("allocation")]] void *palloc(size_t);
[[gnu::nonnull]] void freecl(void *);
[[gnu::nonnull]] void fclosecl(FILE **);
[[gnu::nonnull]] void closedircl(DIR **);
