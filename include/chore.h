#pragma once
#include <dirent.h>
#include <stdio.h>

#define TOSTR(x) #x
#define TO2STR(x) TOSTR(x)
#define HERE __FILE__ ":" TO2STR(__LINE__)
#define ALPN ('z' - 'a' + 1)
#define lesser(lhs, rhs) (lhs) < (rhs) ? (lhs) : (rhs)
#define bigger(lhs, rhs) (lhs) > (rhs) ? (lhs) : (rhs)
#define overloadable __attribute__((overloadable))
#define ondrop(cl) __attribute__((cleanup(cl)))
#define drop ondrop(free_cl)
#define dropfile ondrop(fclose_cl)
#define dropdir ondrop(closedir_cl)
#define likely(cond) __builtin_expect(!!(cond), 1)
#define unlikely(cond) __builtin_expect(!!(cond), 0)

struct winsize get_winsz();
bool isint(double) __attribute__((const));
void skipspcs(char const **);
void skip_untilcomma(char const **);
void nfree(void *);
void *ealloc(int);
void *palloc(int);
void free_cl(void *);
void fclose_cl(FILE **);
void closedir_cl(DIR **);
