#pragma once
#define ALPN ('z' - 'a' + 1)
#define lesser(lhs, rhs) (lhs) < (rhs) ? (lhs) : (rhs)
#define bigger(lhs, rhs) (lhs) > (rhs) ? (lhs) : (rhs)

struct winsize get_winsz();
bool isint(double) __attribute__((const));
void skipspcs(char **);
void skip_untilcomma(char **);
void nfree(void *);
void *ealloc(int);
