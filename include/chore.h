/**
 * @file include/chore.h
 * @brief Define util macros
 */

#pragma once
#include "def.h"
#include <dirent.h>
#include <stdio.h>

constexpr size_t ALPN = 'z' - 'a' + 1;

#define HERE             __FILE__ ":" TOSTR(__LINE__)
#define lesser(lhs, rhs) ((lhs) < (rhs) ? (lhs) : (rhs))
#define bigger(lhs, rhs) ((lhs) > (rhs) ? (lhs) : (rhs))
#define overloadable     [[clang::overloadable]]
#define ondrop(cl)       [[gnu::cleanup(cl)]]
#define drop             ondrop(free_cl)
#define dropfile         ondrop(fclose_cl)
#define dropdir          ondrop(closedir_cl)
#define _                auto CAT(_DISCARD_, __COUNTER__) [[gnu::unused]]

// zig style alloc()
#define zalloc(T, size) ((T *)palloc(size * sizeof(T)))

#define nfree(p) \
  do { \
    if (p == nullptr) break; \
    free(p); \
    p = nullptr; \
  } while (0)

struct winsize get_winsz();
[[gnu::const]] bool isint(double);
[[gnu::nonnull]] overloadable void skipspcs(char const **);
[[gnu::nonnull]] overloadable void skipspcs(char const **, size_t);
[[gnu::nonnull]] void skip_untilcomma(char const **);
[[gnu::returns_nonnull, nodiscard("allocation")]] void *palloc(size_t);
[[gnu::nonnull]] void free_cl(void *);
[[gnu::nonnull]] void fclose_cl(FILE **);
[[gnu::nonnull]] void closedir_cl(DIR **);
