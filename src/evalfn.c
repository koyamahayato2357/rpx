#include "evalfn.h"
#include "arthfn.h"
#include "gene.h"
#include "phyconst.h"
#include "testing.h"
#include <ctype.h>
#include <math.h>

char *expr;

#define PUSH(x) *++*rsp = x
#define POP *(*rsp)--

#define DEF_ARTHMS(tok, op)                                                    \
  void rpx_##tok(double **rbp, double **rsp) {                                 \
    for (; *rbp + 1 < *rsp; *(*rbp + 1) op## = POP)                            \
      ;                                                                        \
  }
APPLY_ARTHM(DEF_ARTHMS)

void rpx_mod(double **rbp, double **rsp) {
  for (; *rbp + 1 < *rsp; *(*rbp + 1) = fmod(*(*rbp + 1), POP))
    ;
}

void rpx_pow(double **rbp, double **rsp) {
  for (; *rbp + 1 < *rsp; *(*rbp + 1) = pow(*(*rbp + 1), POP))
    ;
}

void rpx_eql(double **rbp, double **rsp) {
  for (; *rbp + 1 < *rsp && eq(*(*rsp - 1), **rsp); (*rsp)--)
    ;
  *(*rbp + 1) = *rbp + 1 == *rsp;
  *rsp = *rbp + 1;
}

#define DEF_LTGT(tok, op)                                                      \
  void rpx_##tok(double **rbp, double **rsp) {                                 \
    for (; *rbp + 1 < *rsp && *(*rsp - 1) op * *rsp; (*rsp)--)                 \
      ;                                                                        \
    *(*rbp + 1) = *rbp + 1 == *rsp;                                            \
    *rsp = *rbp + 1;                                                           \
  }
APPLY_LTGT(DEF_LTGT)

#define DEF_ONEARGFN(f)                                                        \
  void rpx_##f(double **rbp [[maybe_unused]], double **rsp) {                  \
    **rsp = f(**rsp);                                                          \
  }
DEF_ONEARGFN(sin)
DEF_ONEARGFN(cos)
DEF_ONEARGFN(tan)
DEF_ONEARGFN(fabs)
DEF_ONEARGFN(tgamma)
DEF_ONEARGFN(ceil)
DEF_ONEARGFN(floor)
DEF_ONEARGFN(round)

#define DEF_MULTI(name, factor)                                                \
  void rpx_##name(double **rbp [[maybe_unused]], double **rsp) {               \
    **rsp *= factor;                                                           \
  }
DEF_MULTI(nagate, -1)
DEF_MULTI(torad, M_PI / 180)
DEF_MULTI(todeg, 180 / M_PI)

#define DEF_TWOCHARFN(name, c1, f1, c2, f2, c3, f3)                            \
  void rpx_##name(double **rbp [[maybe_unused]], double **rsp) {               \
    switch (*++expr) {                                                         \
    case c1:                                                                   \
      **rsp = f1(**rsp);                                                       \
      break;                                                                   \
    case c2:                                                                   \
      **rsp = f2(**rsp);                                                       \
      break;                                                                   \
    case c3:                                                                   \
      **rsp = f3(**rsp);                                                       \
      break;                                                                   \
    }                                                                          \
  }
DEF_TWOCHARFN(hyp, 's', sinh, 'c', cosh, 't', tanh)
DEF_TWOCHARFN(arc, 's', asin, 'c', acos, 't', atan)
DEF_TWOCHARFN(log, '2', log2, 'c', log10, 'e', log)

void rpx_logbase(double **rbp [[maybe_unused]], double **rsp) {
  double x = *(*rsp)--;
  **rsp = log(**rsp) / log(x);
}

void rpx_const(double **rbp [[maybe_unused]], double **rsp) {
  PUSH(get_const(*++expr));
}

void (*eval_table['~' - ' '])(double **, double **) = {
    nullptr,    // '!'
    nullptr,    // '"'
    nullptr,    // '#'
    nullptr,    // '$'
    rpx_mod,    // '%'
    nullptr,    // '&'
    nullptr,    // '''
    nullptr,    // '('
    nullptr,    // ')'
    rpx_mul,    // '*'
    rpx_add,    // '+'
    nullptr,    // ','
    rpx_sub,    // '-'
    nullptr,    // '.'
    rpx_div,    // '/'
    nullptr,    // '0'
    nullptr,    // '1'
    nullptr,    // '2'
    nullptr,    // '3'
    nullptr,    // '4'
    nullptr,    // '5'
    nullptr,    // '6'
    nullptr,    // '7'
    nullptr,    // '8'
    nullptr,    // '9'
    nullptr,    // ':'
    nullptr,    // ';'
    rpx_lt,     // '<'
    rpx_eql,    // '='
    rpx_gt,     // '>'
    nullptr,    // '?'
    nullptr,    // '@'
    rpx_fabs,   // 'A'
    nullptr,    // 'B'
    rpx_ceil,   // 'C'
    nullptr,    // 'D'
    nullptr,    // 'E'
    rpx_floor,  // 'F'
    nullptr,    // 'G'
    nullptr,    // 'H'
    nullptr,    // 'I'
    nullptr,    // 'J'
    nullptr,    // 'K'
    nullptr,    // 'L'
    nullptr,    // 'M'
    nullptr,    // 'N'
    nullptr,    // 'O'
    nullptr,    // 'P'
    nullptr,    // 'Q'
    rpx_round,  // 'R'
    nullptr,    // 'S'
    nullptr,    // 'T'
    nullptr,    // 'U'
    nullptr,    // 'V'
    nullptr,    // 'W'
    nullptr,    // 'X'
    nullptr,    // 'Y'
    nullptr,    // 'Z'
    nullptr,    // '['
    rpx_const,  // '\'
    nullptr,    // ']'
    rpx_pow,    // '^'
    nullptr,    // '_'
    nullptr,    // '`'
    rpx_arc,    // 'a'
    nullptr,    // 'b'
    rpx_cos,    // 'c'
    rpx_todeg,  // 'd'
    nullptr,    // 'e'
    nullptr,    // 'f'
    rpx_tgamma, // 'g'
    rpx_hyp,    // 'h'
    nullptr,    // 'i'
    nullptr,    // 'j'
    nullptr,    // 'k'
    rpx_log,    // 'l'
    rpx_nagate, // 'm'
    nullptr,    // 'n'
    nullptr,    // 'o'
    nullptr,    // 'p'
    nullptr,    // 'q'
    rpx_torad,  // 'r'
    rpx_sin,    // 's'
    rpx_tan,    // 't'
    nullptr,    // 'u'
    nullptr,    // 'v'
    nullptr,    // 'w'
    nullptr,    // 'x'
    nullptr,    // 'y'
    nullptr,    // 'z'
    nullptr,    // '{'
    nullptr,    // '|'
    nullptr,    // '}'
    nullptr,    // '~'
};

double temp_eval(char *a_expr) {
  expr = a_expr;
  double stack[100];
  double *rbp = stack, *rsp = stack;
  for (; *expr; expr++) {
    if (isdigit(*expr))
      *++rsp = strtod(expr, &expr);
    else if (isspace(*expr))
      continue;
    else
      eval_table[*expr - ' ' - 1](&rbp, &rsp);
  }
  return *rsp;
}

test_table(temp, temp_eval, (double, char *),
           {{2, "1 1 +"},
            {5, "10 2 /"},
            {15.0, "1 2 3 4 5 +"},
            {1024.0, "4 5 ^"},
            {1.0, "\\P 2 / s"},
            {2.0, "100 lc"}})
