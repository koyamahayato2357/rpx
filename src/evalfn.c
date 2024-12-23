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
  case 'd': // display
    printany(*ei->rsp);
    putchar('\n');
    break;
  case 'h':
    *ei->rsp = ei->info.hist[ei->info.histi - (int)*ei->rsp - 1].elem.real;
    break;
  case 'n':
    PUSH = SNAN;
    break;
  case 'p':
    ei->rsp[1] = *ei->rsp;
    ei->rsp++;
    break;
  case 'r':
    PUSH = rand() / (double)RAND_MAX;
    break;
  case 's':
    *ei->rsp = *(ei->rsp - (int)*ei->rsp - 1);
    break;
  }
}

static double handle_function_args(evalinfo_t *ei) {
  int argnum = *ei->expr - '0';
  if (ei->max_argc[ei->max_argci] < argnum)
    ei->max_argc[ei->max_argci] = argnum;
  return ei->info.usrfn.argv[8 - argnum];
}

static void rpx_lvars(evalinfo_t *ei) {
  PUSH = (isdigit(*++ei->expr)) ? handle_function_args(ei)
         : (islower(*ei->expr)) ? ei->info.usrvar[*ei->expr - 'a'].elem.real
                                : $panic(ERR_CHAR_NOT_FOUND);
}

static void rpx_wvars(evalinfo_t *ei) {
  ei->info.usrvar[*++ei->expr - 'a'].elem.real = *ei->rsp;
}

static void rpx_end(evalinfo_t *ei) { ei->iscontinue = false; }

static void rpx_grpbgn(evalinfo_t *ei) {
  *++*(double ***)&ei->rsp = ei->rbp;
  ei->rbp = ei->rsp;
}

static void rpx_grpend(evalinfo_t *ei) {
  double ret = *ei->rsp;
  ei->rsp = ei->rbp;
  ei->rbp = *(double **)ei->rbp;
  *ei->rsp = ret;
}

static void rpx_lmdbgn(evalinfo_t *ei) {
  rpx_grpbgn(ei);
  memcpy(ei->callstack[++ei->callstacki], ei->info.usrfn.argv,
         8 * sizeof(double));
  memcpy(ei->info.usrfn.argv, ei->rsp - 8, 8 * sizeof(double));
  set_rtinfo('r', ei->info);
  ei->max_argc[++ei->max_argci] = 0;
}

static void rpx_lmdend(evalinfo_t *ei) {
  rpx_grpend(ei);
  memcpy(ei->info.usrfn.argv, ei->callstack[ei->callstacki--],
         8 * sizeof(double));
  set_rtinfo('r', ei->info);
  double ret = *ei->rsp;
  ei->rsp -= ei->max_argc[ei->max_argci];
  *ei->rsp = ret;
}

static void rpx_callfn(evalinfo_t *ei) {
  int fname = *++ei->expr - 'a';
  char const *temp = ei->expr;
  ei->expr = ei->info.usrfn.expr[fname];
  rpx_lmdbgn(ei);
  *ei->rsp = eval_expr_real_with_info(ei).elem.real;
  rpx_lmdend(ei);
  ei->expr = temp;
}

static void rpx_undfned(evalinfo_t *ei) {
  _ = ei;
  throw(ERR_UNKNOWN_CHAR);
}

void (*eval_table['~' - ' ' + 1])(evalinfo_t *) = {
    rpx_space,   // ' '
    rpx_callfn,  // '!'
    rpx_undfned, // '"'
    rpx_undfned, // '#'
    rpx_lvars,   // '$'
    rpx_mod,     // '%'
    rpx_wvars,   // '&'
    rpx_undfned, // '''
    rpx_grpbgn,  // '('
    rpx_grpend,  // ')'
    rpx_mul,     // '*'
    rpx_add,     // '+'
    rpx_end,     // ','
    rpx_sub,     // '-'
    rpx_undfned, // '.'
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
    rpx_undfned, // ':'
    rpx_end,     // ';'
    rpx_lt,      // '<'
    rpx_eql,     // '='
    rpx_gt,      // '>'
    rpx_undfned, // '?'
    rpx_sysfn,   // '@'
    rpx_fabs,    // 'A'
    rpx_undfned, // 'B'
    rpx_ceil,    // 'C'
    rpx_undfned, // 'D'
    rpx_undfned, // 'E'
    rpx_floor,   // 'F'
    rpx_undfned, // 'G'
    rpx_undfned, // 'H'
    rpx_undfned, // 'I'
    rpx_undfned, // 'J'
    rpx_undfned, // 'K'
    rpx_logbase, // 'L'
    rpx_undfned, // 'M'
    rpx_undfned, // 'N'
    rpx_undfned, // 'O'
    rpx_undfned, // 'P'
    rpx_undfned, // 'Q'
    rpx_round,   // 'R'
    rpx_undfned, // 'S'
    rpx_undfned, // 'T'
    rpx_undfned, // 'U'
    rpx_undfned, // 'V'
    rpx_undfned, // 'W'
    rpx_undfned, // 'X'
    rpx_undfned, // 'Y'
    rpx_undfned, // 'Z'
    rpx_undfned, // '['
    rpx_const,   // '\'
    rpx_undfned, // ']'
    rpx_pow,     // '^'
    rpx_undfned, // '_'
    rpx_undfned, // '`'
    rpx_arc,     // 'a'
    rpx_undfned, // 'b'
    rpx_cos,     // 'c'
    rpx_todeg,   // 'd'
    rpx_undfned, // 'e'
    rpx_undfned, // 'f'
    rpx_tgamma,  // 'g'
    rpx_hyp,     // 'h'
    rpx_intfn,   // 'i'
    rpx_undfned, // 'j'
    rpx_undfned, // 'k'
    rpx_log,     // 'l'
    rpx_nagate,  // 'm'
    rpx_undfned, // 'n'
    rpx_undfned, // 'o'
    rpx_undfned, // 'p'
    rpx_undfned, // 'q'
    rpx_torad,   // 'r'
    rpx_sin,     // 's'
    rpx_tan,     // 't'
    rpx_undfned, // 'u'
    rpx_undfned, // 'v'
    rpx_undfned, // 'w'
    rpx_undfned, // 'x'
    rpx_undfned, // 'y'
    rpx_undfned, // 'z'
    rpx_lmdbgn,  // '{'
    rpx_undfned, // '|'
    rpx_lmdend,  // '}'
    rpx_undfned, // '~'
};

void (*get_eval_table(char c))(evalinfo_t *) { return eval_table[c - ' ']; }

elem_t eval_expr_real_with_info(evalinfo_t *ei) {
  for (; likely(*ei->expr && ei->iscontinue); ei->expr++)
    get_eval_table (*ei->expr)(ei);
  if (ei->info.histi < BUFSIZE)
    ei->info.hist[ei->info.histi++].elem.real = *ei->rsp;
  set_rtinfo('r', ei->info);
  return (elem_t){.rtype = RTYPE_REAL, .elem = {.real = *ei->rsp}};
}

/**
 * @brief Evaluate real number expression
 * @param a_expr String of expression
 * @return Expression evaluation result
 */
elem_t eval_expr_real(char const *a_expr) {
  evalinfo_t ei;
  ei.rbp = ei.rsp = ei.stack - 1;
  ei.info = get_rtinfo('r');
  ei.expr = a_expr;
  ei.iscontinue = true;
  ei.callstacki = ~0;
  return eval_expr_real_with_info(&ei);
}

test(eval_expr_real) {
  expecteq(11.0, eval_expr_real("5 6 + &x").elem.real);
  expecteq(22.0, eval_expr_real("$x 2 *").elem.real);

  // function
  proc_cmds("df$1$1+");
  expecteq(10.0, eval_expr_real("5!f").elem.real);

  // nest group
  expecteq(33.0, eval_expr_real("4 5 (5 6 (6 7 +) +) +").elem.real);

  // lambda
  expecteq(8.0, eval_expr_real("4 {$1 2 *}").elem.real);
  expecteq(
      19.0,
      eval_expr_real("1 5 {$1 3 +} {5 $1 * {$1 4 -} {$1 2 /} $2 +}").elem.real);
}

bench(eval_expr_real) {
  eval_expr_real("1 2 3 4 5 +");
  eval_expr_real("4 5 ^");
  eval_expr_real("1s2^(1c2^)+");
  eval_expr_real("  5    6    10    - 5  /");
  eval_expr_real("5");
  eval_expr_real("@a");
  eval_expr_real("10 &x");
  eval_expr_real("$x 2 *");
  eval_expr_real("2 3 ^ (4 5 *) + (6 7 /) -");
  eval_expr_real("\\P 2 / s");
  eval_expr_real("\\P 4 / c");
  eval_expr_real("2 l2");
  eval_expr_real("100 lc");
  eval_expr_real("1 0 /");
}
