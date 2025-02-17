#pragma once
#include <dirent.h>
#include <stdio.h>

#define EXPAND(...) __VA_ARGS__
#define CAT(a, b) CAT2(a, b)
#define CAT2(a, b) a##b
#define TOSTR(x) #x
#define TO2STR(x) TOSTR(x)
#define HERE __FILE__ ":" TO2STR(__LINE__)
#define ALPN ('z' - 'a' + 1)
#define lesser(lhs, rhs) (lhs) < (rhs) ? (lhs) : (rhs)
#define bigger(lhs, rhs) (lhs) > (rhs) ? (lhs) : (rhs)
#define overloadable [[clang::overloadable]]
#define ondrop(cl) [[gnu::cleanup(cl)]]
#define drop ondrop(free_cl)
#define dropfile ondrop(fclose_cl)
#define dropdir ondrop(closedir_cl)
#define _ auto CAT(_DISCARD_, __COUNTER__) [[maybe_unused]]

struct winsize get_winsz();
[[gnu::const]] bool isint(double);
void skipspcs(char const **);
void skip_untilcomma(char const **);
void nfree(void *);
void *palloc(int);
void free_cl(void *);
void fclose_cl(FILE **);
void closedir_cl(DIR **);
