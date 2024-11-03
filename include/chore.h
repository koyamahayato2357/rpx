#pragma once
#define TOSTR(x) #x
#define TO2STR(x) TOSTR(x)
#define HERE __FILE__ ":" TO2STR(__LINE__)
#define ALPN ('z' - 'a' + 1)
#define lesser(lhs, rhs) (lhs) < (rhs) ? (lhs) : (rhs)
#define bigger(lhs, rhs) (lhs) > (rhs) ? (lhs) : (rhs)
#define overloadable __attribute__((overloadable))

struct winsize get_winsz();
bool isint(double) __attribute__((const));
void skipspcs(char **);
void skip_untilcomma(char **);
void nfree(void *);
void *ealloc(int);
