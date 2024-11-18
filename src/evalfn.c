#include "evalfn.h"
#include "arthfn.h"
#include "benchmarking.h"
#include "chore.h"
#include "errcode.h"
#include "exproriented.h"
#include "gene.h"
#include "main.h"
#include "phyconst.h"
#include "string.h"
#include "sysconf.h"
#include "testing.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

char const *expr;
rtinfo_t info;
double *rbp, *rsp;

elem_t eval_expr_real(char const *);

#define PUSH *++rsp
#define POP *rsp--

#define DEF_ARTHMS(tok, op)                                                    \
  void rpx_##tok() {                                                           \
    for (; rbp + 1 < rsp; rbp[1] op## = POP)                                   \
      ;                                                                        \
  }
APPLY_ARTHM(DEF_ARTHMS)

void rpx_mod() {
  for (; rbp + 1 < rsp; rbp[1] = fmod(rbp[1], POP))
    ;
}

void rpx_pow() {
  for (; rbp + 1 < rsp; rbp[1] = pow(rbp[1], POP))
    ;
}

void rpx_eql() {
  for (; rbp + 1 < rsp && eq(rsp[-1], *rsp); POP)
    ;
  rbp[1] = rbp + 1 == rsp;
  rsp = rbp + 1;
}

#define DEF_LTGT(tok, op)                                                      \
  void rpx_##tok() {                                                           \
    for (; rbp + 1 < rsp && rsp[-1] op * rsp; POP)                             \
      ;                                                                        \
    rbp[1] = rbp + 1 == rsp;                                                   \
    rsp = rbp + 1;                                                             \
  }
APPLY_LTGT(DEF_LTGT)

#define DEF_ONEARGFN(f)                                                        \
  void rpx_##f() { *rsp = f(*rsp); }
DEF_ONEARGFN(sin)
DEF_ONEARGFN(cos)
DEF_ONEARGFN(tan)
DEF_ONEARGFN(fabs)
DEF_ONEARGFN(tgamma)
DEF_ONEARGFN(ceil)
DEF_ONEARGFN(floor)
DEF_ONEARGFN(round)

#define DEF_MULTI(name, factor)                                                \
  void rpx_##name() { *rsp *= factor; }
DEF_MULTI(nagate, -1)
DEF_MULTI(torad, M_PI / 180)
DEF_MULTI(todeg, 180 / M_PI)

#define DEF_TWOCHARFN(name, c1, f1, c2, f2, c3, f3)                            \
  void rpx_##name() {                                                          \
    switch (*++expr) {                                                         \
      OVERWRITE_REAL(c1, f1)                                                   \
      OVERWRITE_REAL(c2, f2)                                                   \
      OVERWRITE_REAL(c3, f3)                                                   \
    }                                                                          \
  }
DEF_TWOCHARFN(hyp, 's', sinh, 'c', cosh, 't', tanh)
DEF_TWOCHARFN(arc, 's', asin, 'c', acos, 't', atan)
DEF_TWOCHARFN(log, '2', log2, 'c', log10, 'e', log)

void rpx_logbase() {
  double x = POP;
  *rsp = log(*rsp) / log(x);
}

void rpx_const() { PUSH = get_const(*++expr); }

void rpx_parse() {
  PUSH = strtod(expr, (char **)&expr);
  expr--;
}

void rpx_space() {
  skipspcs((char const **)&expr);
  expr--;
}

#define CASE_TWOARGFN(c, f)                                                    \
  case c: {                                                                    \
    double x = *rsp;                                                           \
    *rsp = f(*rsp, x);                                                         \
  } break;
void rpx_intfn() {
  switch (*++expr) {
    CASE_TWOARGFN('g', gcd)
    CASE_TWOARGFN('l', lcm)
    CASE_TWOARGFN('p', permutation)
    CASE_TWOARGFN('c', combination)
  }
}

void rpx_sysfn() {
  switch (*++expr) {
  case 'a': // ANS
    PUSH = info.hist[info.histi - 1].elem.real;
    break;
  case 'h':
    *rsp = info.hist[info.histi - (int)*rsp - 1].elem.real;
    break;
  case 'p':
    rsp[1] = *rsp;
    rsp++;
    break;
  case 's':
    *rsp = *(rsp - (int)*rsp - 1);
    break;
  case 'r':
    *rsp = rand() / (double)RAND_MAX;
    break;
  }
}

void rpx_callfn() {
  int fname = *expr - 'a';
  *rsp -= info.usrfn.argc[fname] - 1;
  memcpy(info.usrfn.argv, rsp, info.usrfn.argc[fname] * sizeof(double));
  set_rtinfo('r', info);
  *rsp = eval_expr_real(info.usrfn.expr[fname]).elem.real;
}

void rpx_lvars() {
  PUSH = (isdigit(*++expr)) ? info.usrfn.argv[*expr - '0' - 1]
         : (islower(*expr)) ? info.usrvar[*expr - 'a'].elem.real
                            : $panic(ERR_CHAR_NOT_FOUND);
}

void rpx_wvars() { info.usrvar[*++expr - 'a'].elem.real = *rsp; }

void rpx_end() { ((char *)expr)[1] = '\0'; }

void rpx_grpbgn() {
  *++*(long **)&rsp = (long)rbp;
  rbp = rsp;
}

void rpx_grpend() {
  rbp = *(double **)rbp;
  rsp--;
  *rsp = rsp[1];
}

void (*eval_table['~' - ' ' + 1])() = {
    rpx_space,  // ' '
    rpx_callfn, // '!'
    nullptr,    // '"'
    nullptr,    // '#'
    rpx_lvars,  // '$'
    rpx_mod,    // '%'
    rpx_wvars,  // '&'
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

/**
 * @brief Evaluate real number expression
 * @param expr String of expression
 * @return Expression evaluation result
 */
elem_t eval_expr_real(char const *a_expr) {
  expr = a_expr;
  info = get_rtinfo('r');
  double stack[100];
  rbp = stack;
  rsp = stack;
  for (; *expr; expr++)
    eval_table[*expr - ' ']();
  if (info.histi < BUFSIZE)
    info.hist[info.histi++].elem.real = *rsp;
  set_rtinfo('r', info);
  return (elem_t){.rtype = RTYPE_REAL, .elem = {.real = *rsp}};
}

test(eval_expr_real) {
  expecteq(11.0, eval_expr_real("5 6 + &x").elem.real);
  expecteq(22.0, eval_expr_real("$x 2 *").elem.real);
}

bench(eval_expr_real) {
  eval_expr_real("1 2 3 4 5 +");
  eval_expr_real("4 5 ^");
  eval_expr_real("1s2^(1c2^)+");
  eval_expr_real("  5    6    10    - 5  /");

  // Test ANS functionality
  eval_expr_real("5");
  eval_expr_real("@a");

  // Test variable operations
  eval_expr_real("10 &x");
  eval_expr_real("$x 2 *");

  // Test more complex expressions
  eval_expr_real("2 3 ^ (4 5 *) + (6 7 /) -");

  // Test trigonometric functions
  eval_expr_real("\\P 2 / s");
  eval_expr_real("\\P 4 / c");

  // Test logarithmic functions
  eval_expr_real("2 l2");
  eval_expr_real("100 lc");

  // Test error handling
  eval_expr_real("1 0 /");
}
