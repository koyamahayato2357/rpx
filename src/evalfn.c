#include "evalfn.h"
#include "arthfn.h"
#include "benchmarking.h"
#include "chore.h"
#include "gene.h"
#include "phyconst.h"
#include "string.h"
#include "sysconf.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

char *expr;
rtinfo_t info;

double temp_eval(char *);

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

void rpx_parse(double **rbp [[maybe_unused]], double **rsp) {
  PUSH(strtod(expr, &expr));
  expr--;
}

void rpx_space(double **rbp [[maybe_unused]], double **rsp [[maybe_unused]]) {
  skipspcs((char const **)&expr);
}

#define CASE_TWOARGFN(c, f)                                                    \
  case c: {                                                                    \
    double x = **rsp;                                                          \
    **rsp = f(**rsp, x);                                                       \
  } break;
void rpx_intfn(double **rbp [[maybe_unused]], double **rsp) {
  switch (*++expr) {
    CASE_TWOARGFN('g', gcd)
    CASE_TWOARGFN('l', lcm)
    CASE_TWOARGFN('p', permutation)
    CASE_TWOARGFN('c', combination)
  }
}

void rpx_sysfn(double **rbp [[maybe_unused]], double **rsp) {
  switch (*++expr) {
  case 'a': // ANS
    *++*rsp = info.hist[info.histi - 1].elem.real;
    break;
  case 'h':
    **rsp = info.hist[info.histi - (int)**rsp - 1].elem.real;
    break;
  case 'p':
    (*rsp)++;
    **rsp = *(*rsp - 1);
    break;
  case 's':
    **rsp = *(*rsp - (int)**rsp - 1);
    break;
  }
}

void rpx_callfn(double **rbp [[maybe_unused]], double **rsp) {
  int fname = *expr - 'a';
  *rsp -= info.usrfn.argc[fname] - 1;
  memcpy(info.usrfn.argv, *rsp, info.usrfn.argc[fname] * sizeof(double));
  set_rtinfo('r', info);
  **rsp = temp_eval(info.usrfn.expr[fname]);
}

void rpx_vars(double **rbp [[maybe_unused]], double **rsp) {
  if (islower(*++expr)) {
    char vname = *expr++;
    skipspcs((char const **)&expr);
    if (*expr == 'u')
      info.usrvar[vname - 'a'].elem.real = **rsp;
    else {
      *++*rsp = info.usrvar[vname - 'a'].elem.real;
      expr--;
    }
  } else if (isdigit(*expr))
    *++*rsp = info.usrfn.argv[*expr - '0' - 1];
  else
    switch (*expr) {
    case 'R':
      *++*rsp = rand() / (double)RAND_MAX;
      break;
    }
}

void rpx_end(double **rbp [[maybe_unused]], double **rsp [[maybe_unused]]) {
  expr[1] = '\0';
}

void rpx_grpbgn(double **rbp, double **rsp) {
  *++*rsp = (double)(long)*rbp;
  *rbp = *rsp;
}

void rpx_grpend(double **rbp, double **rsp) {
  *rbp = (double *)(long)**rbp;
  (*rsp)--;
  **rsp = *(*rsp + 1);
}

void (*eval_table['~' - ' ' + 1])(double **, double **) = {
    rpx_space,  // ' '
    rpx_callfn, // '!'
    nullptr,    // '"'
    nullptr,    // '#'
    rpx_vars,   // '$'
    rpx_mod,    // '%'
    nullptr,    // '&'
    nullptr,    // '''
    rpx_grpbgn, // '('
    rpx_grpend, // ')'
    rpx_mul,    // '*'
    rpx_add,    // '+'
    rpx_end,    // ','
    rpx_sub,    // '-'
    nullptr,    // '.'
    rpx_div,    // '/'
    rpx_parse,  // '0'
    rpx_parse,  // '1'
    rpx_parse,  // '2'
    rpx_parse,  // '3'
    rpx_parse,  // '4'
    rpx_parse,  // '5'
    rpx_parse,  // '6'
    rpx_parse,  // '7'
    rpx_parse,  // '8'
    rpx_parse,  // '9'
    nullptr,    // ':'
    rpx_end,    // ';'
    rpx_lt,     // '<'
    rpx_eql,    // '='
    rpx_gt,     // '>'
    nullptr,    // '?'
    rpx_sysfn,  // '@'
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
    rpx_intfn,  // 'i'
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
  info = get_rtinfo('r');
  double stack[100];
  double *rbp = stack, *rsp = stack;
  for (; *expr; expr++)
    eval_table[*expr - ' '](&rbp, &rsp);
  if (info.histi < BUFSIZE)
    info.hist[info.histi++].elem.real = *rsp;
  set_rtinfo('r', info);
  return *rsp;
}

bench(temp_eval) {
  temp_eval("1 2 3 4 5 +");
  temp_eval("4 5 ^");
  temp_eval("1s2^(1c2^)+");
  temp_eval("  5    6    10    - 5  /");

  // Test ANS functionality
  temp_eval("5");
  temp_eval("@a");

  // Test variable operations
  temp_eval("10 $x u");
  temp_eval("$x 2 *");

  // Test more complex expressions
  temp_eval("2 3 ^ (4 5 *) + (6 7 /) -");

  // Test trigonometric functions
  temp_eval("\\P 2 / s");
  temp_eval("\\P 4 / c");

  // Test logarithmic functions
  temp_eval("2 l2");
  temp_eval("100 lc");

  // Test error handling
  temp_eval("1 0 /");
}
