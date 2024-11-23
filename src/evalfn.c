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

elem_t eval_expr_real(char const *);

#define PUSH *++ei->rsp
#define POP *ei->rsp--

#define DEF_ARTHMS(tok, op)                                                    \
  static void rpx_##tok(evalinfo_t *ei) {                                      \
    for (; ei->rbp + 1 < ei->rsp; ei->rbp[1] op## = POP)                       \
      ;                                                                        \
  }
APPLY_ARTHM(DEF_ARTHMS)

static void rpx_mod(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp; ei->rbp[1] = fmod(ei->rbp[1], POP))
    ;
}

static void rpx_pow(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp; ei->rbp[1] = pow(ei->rbp[1], POP))
    ;
}

static void rpx_eql(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp && eq(ei->rsp[-1], *ei->rsp); POP)
    ;
  ei->rbp[1] = ei->rbp + 1 == ei->rsp;
  ei->rsp = ei->rbp + 1;
}

#define DEF_LTGT(tok, op)                                                      \
  static void rpx_##tok(evalinfo_t *ei) {                                      \
    for (; ei->rbp + 1 < ei->rsp && ei->rsp[-1] op * ei->rsp; POP)             \
      ;                                                                        \
    ei->rbp[1] = ei->rbp + 1 == ei->rsp;                                       \
    ei->rsp = ei->rbp + 1;                                                     \
  }
APPLY_LTGT(DEF_LTGT)

#define DEF_ONEARGFN(f)                                                        \
  static void rpx_##f(evalinfo_t *ei) { *ei->rsp = f(*ei->rsp); }
DEF_ONEARGFN(sin)
DEF_ONEARGFN(cos)
DEF_ONEARGFN(tan)
DEF_ONEARGFN(fabs)
DEF_ONEARGFN(tgamma)
DEF_ONEARGFN(ceil)
DEF_ONEARGFN(floor)
DEF_ONEARGFN(round)

#define DEF_MULTI(name, factor)                                                \
  static void rpx_##name(evalinfo_t *ei) { *ei->rsp *= factor; }
DEF_MULTI(nagate, -1)
DEF_MULTI(torad, M_PI / 180)
DEF_MULTI(todeg, 180 / M_PI)

#define DEF_TWOCHARFN(name, c1, f1, c2, f2, c3, f3)                            \
  static void rpx_##name(evalinfo_t *ei) {                                     \
    switch (*++ei->expr) {                                                     \
      OVERWRITE_REAL(c1, f1)                                                   \
      OVERWRITE_REAL(c2, f2)                                                   \
      OVERWRITE_REAL(c3, f3)                                                   \
    }                                                                          \
  }
DEF_TWOCHARFN(hyp, 's', sinh, 'c', cosh, 't', tanh)
DEF_TWOCHARFN(arc, 's', asin, 'c', acos, 't', atan)
DEF_TWOCHARFN(log, '2', log2, 'c', log10, 'e', log)

static void rpx_logbase(evalinfo_t *ei) {
  double x = POP;
  *ei->rsp = log(*ei->rsp) / log(x);
}

static void rpx_const(evalinfo_t *ei) { PUSH = get_const(*++ei->expr); }

static void rpx_parse(evalinfo_t *ei) {
  char *next = nullptr;
  PUSH = strtod(ei->expr, &next);
  ei->expr = next - 1;
}

static void rpx_space(evalinfo_t *ei) {
  skipspcs(&ei->expr);
  ei->expr--;
}

#define CASE_TWOARGFN(c, f)                                                    \
  case c: {                                                                    \
    double x = *ei->rsp;                                                       \
    *ei->rsp = f(*ei->rsp, x);                                                 \
  } break;
static void rpx_intfn(evalinfo_t *ei) {
  switch (*++ei->expr) {
    CASE_TWOARGFN('g', gcd)
    CASE_TWOARGFN('l', lcm)
    CASE_TWOARGFN('p', permutation)
    CASE_TWOARGFN('c', combination)
  }
}

static void rpx_sysfn(evalinfo_t *ei) {
  switch (*++ei->expr) {
  case 'a': // ANS
    PUSH = ei->info.hist[ei->info.histi - 1].elem.real;
    break;
  case 'h':
    *ei->rsp = ei->info.hist[ei->info.histi - (int)*ei->rsp - 1].elem.real;
    break;
  case 'p':
    ei->rsp[1] = *ei->rsp;
    ei->rsp++;
    break;
  case 's':
    *ei->rsp = *(ei->rsp - (int)*ei->rsp - 1);
    break;
  case 'r':
    *ei->rsp = rand() / (double)RAND_MAX;
    break;
  }
}

static void rpx_callfn(evalinfo_t *ei) {
  int fname = *++ei->expr - 'a';
  int argc = ei->info.usrfn.argc[fname];
  *ei->rsp -= argc - 1;
  memcpy(ei->info.usrfn.argv, ei->rsp, argc * sizeof(double));
  set_rtinfo('r', ei->info);
  *ei->rsp = eval_expr_real(ei->info.usrfn.expr[fname]).elem.real;
}

static void rpx_lvars(evalinfo_t *ei) {
  PUSH = (isdigit(*++ei->expr)) ? ei->info.usrfn.argv[*ei->expr - '0' - 1]
         : (islower(*ei->expr)) ? ei->info.usrvar[*ei->expr - 'a'].elem.real
                                : $panic(ERR_CHAR_NOT_FOUND);
}

static void rpx_wvars(evalinfo_t *ei) {
  ei->info.usrvar[*++ei->expr - 'a'].elem.real = *ei->rsp;
}

static void rpx_end(evalinfo_t *ei) { ((char *)ei->expr)[1] = '\0'; }

static void rpx_grpbgn(evalinfo_t *ei) {
  *++*(long **)&ei->rsp = (long)ei->rbp;
  ei->rbp = ei->rsp;
}

static void rpx_grpend(evalinfo_t *ei) {
  ei->rbp = *(double **)ei->rbp;
  ei->rsp--;
  *ei->rsp = ei->rsp[1];
}

void (*eval_table['~' - ' ' + 1])(evalinfo_t *) = {
    rpx_space,   // ' '
    rpx_callfn,  // '!'
    nullptr,     // '"'
    nullptr,     // '#'
    rpx_lvars,   // '$'
    rpx_mod,     // '%'
    rpx_wvars,   // '&'
    nullptr,     // '''
    rpx_grpbgn,  // '('
    rpx_grpend,  // ')'
    rpx_mul,     // '*'
    rpx_add,     // '+'
    rpx_end,     // ','
    rpx_sub,     // '-'
    nullptr,     // '.'
    rpx_div,     // '/'
    rpx_parse,   // '0'
    rpx_parse,   // '1'
    rpx_parse,   // '2'
    rpx_parse,   // '3'
    rpx_parse,   // '4'
    rpx_parse,   // '5'
    rpx_parse,   // '6'
    rpx_parse,   // '7'
    rpx_parse,   // '8'
    rpx_parse,   // '9'
    nullptr,     // ':'
    rpx_end,     // ';'
    rpx_lt,      // '<'
    rpx_eql,     // '='
    rpx_gt,      // '>'
    nullptr,     // '?'
    rpx_sysfn,   // '@'
    rpx_fabs,    // 'A'
    nullptr,     // 'B'
    rpx_ceil,    // 'C'
    nullptr,     // 'D'
    nullptr,     // 'E'
    rpx_floor,   // 'F'
    nullptr,     // 'G'
    nullptr,     // 'H'
    nullptr,     // 'I'
    nullptr,     // 'J'
    nullptr,     // 'K'
    rpx_logbase, // 'L'
    nullptr,     // 'M'
    nullptr,     // 'N'
    nullptr,     // 'O'
    nullptr,     // 'P'
    nullptr,     // 'Q'
    rpx_round,   // 'R'
    nullptr,     // 'S'
    nullptr,     // 'T'
    nullptr,     // 'U'
    nullptr,     // 'V'
    nullptr,     // 'W'
    nullptr,     // 'X'
    nullptr,     // 'Y'
    nullptr,     // 'Z'
    nullptr,     // '['
    rpx_const,   // '\'
    nullptr,     // ']'
    rpx_pow,     // '^'
    nullptr,     // '_'
    nullptr,     // '`'
    rpx_arc,     // 'a'
    nullptr,     // 'b'
    rpx_cos,     // 'c'
    rpx_todeg,   // 'd'
    nullptr,     // 'e'
    nullptr,     // 'f'
    rpx_tgamma,  // 'g'
    rpx_hyp,     // 'h'
    rpx_intfn,   // 'i'
    nullptr,     // 'j'
    nullptr,     // 'k'
    rpx_log,     // 'l'
    rpx_nagate,  // 'm'
    nullptr,     // 'n'
    nullptr,     // 'o'
    nullptr,     // 'p'
    nullptr,     // 'q'
    rpx_torad,   // 'r'
    rpx_sin,     // 's'
    rpx_tan,     // 't'
    nullptr,     // 'u'
    nullptr,     // 'v'
    nullptr,     // 'w'
    nullptr,     // 'x'
    nullptr,     // 'y'
    nullptr,     // 'z'
    nullptr,     // '{'
    nullptr,     // '|'
    nullptr,     // '}'
    nullptr,     // '~'
};

void (*get_eval_table(char c))(evalinfo_t *) { return eval_table[c - ' ']; }

/**
 * @brief Evaluate real number expression
 * @param a_expr String of expression
 * @return Expression evaluation result
 */
elem_t eval_expr_real(char const *a_expr) {
  evalinfo_t ei;
  ei.rbp = ei.rsp = ei.stack;
  ei.info = get_rtinfo('r');
  ei.expr = a_expr;
  for (; *ei.expr; ei.expr++)
    get_eval_table (*ei.expr)(&ei);
  if (ei.info.histi < BUFSIZE)
    ei.info.hist[ei.info.histi++].elem.real = *ei.rsp;
  set_rtinfo('r', ei.info);
  return (elem_t){.rtype = RTYPE_REAL, .elem = {.real = *ei.rsp}};
}

test(eval_expr_real) {
  expecteq(11.0, eval_expr_real("5 6 + &x").elem.real);
  expecteq(22.0, eval_expr_real("$x 2 *").elem.real);

  // function
  proc_cmds("df1$1$1+");
  expecteq(10.0, eval_expr_real("5!f").elem.real);
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
