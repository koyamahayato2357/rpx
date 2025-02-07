#include "evalfn.h"
#include "arthfn.h"
#include "benchmarking.h"
#include "error.h"
#include "exproriented.h"
#include "phyconst.h"
#include "testing.h"
#include <ctype.h>
#include <math.h>
#include <string.h>

elem_t eval_expr_real(char const *);

#define PUSH (*++ei->rsp)
#define POP (*ei->rsp--)

#define SET_REAL(v)                                                            \
  (real_t) { .elem = {.real = v}, .isnum = true }
#define SET_LAMB(v)                                                            \
  (real_t) { .elem = {.lamb = v}, .isnum = false }

#define DEF_ARTHMS(tok, op)                                                    \
  static void rpx_##tok(evalinfo_t *ei) {                                      \
    for (; ei->rbp + 1 < ei->rsp; ei->rbp[1].elem.real op## = POP.elem.real)   \
      ;                                                                        \
  }
APPLY_ARTHM(DEF_ARTHMS)

static void rpx_mod(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp;
       ei->rbp[1].elem.real = fmod(ei->rbp[1].elem.real, POP.elem.real))
    ;
}

static void rpx_pow(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp;
       ei->rbp[1].elem.real = pow(ei->rbp[1].elem.real, POP.elem.real))
    ;
}

static void rpx_eql(evalinfo_t *ei) {
  for (; ei->rbp + 1 < ei->rsp && eq(ei->rsp[-1].elem.real, ei->rsp->elem.real);
       POP)
    ;
  ei->rbp[1].elem.real = ei->rbp + 1 == ei->rsp ?: SNAN;
  ei->rsp = ei->rbp + 1;
}

#define DEF_LTGT(tok, op)                                                      \
  static void rpx_##tok(evalinfo_t *ei) {                                      \
    for (;                                                                     \
         ei->rbp + 1 < ei->rsp && ei->rsp[-1].elem.real op ei->rsp->elem.real; \
         POP)                                                                  \
      ;                                                                        \
    ei->rbp[1].elem.real = ei->rbp + 1 == ei->rsp ?: SNAN;                     \
    ei->rsp = ei->rbp + 1;                                                     \
  }
APPLY_LTGT(DEF_LTGT)

#define DEF_ONEARGFN(f)                                                        \
  static void rpx_##f(evalinfo_t *ei) {                                        \
    ei->rsp->elem.real = f(ei->rsp->elem.real);                                \
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
  static void rpx_##name(evalinfo_t *ei) { ei->rsp->elem.real *= factor; }
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
  double x = POP.elem.real;
  ei->rsp->elem.real = log(ei->rsp->elem.real) / log(x);
}

static void rpx_const(evalinfo_t *ei) {
  PUSH = SET_REAL(get_const(*++ei->expr));
}

static void rpx_parse(evalinfo_t *ei) {
  char *next = nullptr;
  PUSH = SET_REAL(strtod(ei->expr, &next));
  ei->expr = next - 1;
}

static void rpx_space(evalinfo_t *ei) {
  skipspcs(&ei->expr);
  ei->expr--;
}

#define CASE_TWOARGFN(c, f)                                                    \
  case c: {                                                                    \
    double x = ei->rsp->elem.real;                                             \
    ei->rsp->elem.real = f(ei->rsp->elem.real, x);                             \
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
    PUSH = ei->info.hist[ei->info.histi - 1];
    break;
  case 'd': // display
    printany(ei->rsp->elem.real);
    putchar('\n');
    break;
  case 'h':
    ei->rsp->elem.real =
        ei->info.hist[ei->info.histi - (int)ei->rsp->elem.real - 1].elem.real;
    break;
  case 'n':
    PUSH = SET_REAL(SNAN);
    break;
  case 'p':
    ei->rsp[1] = *ei->rsp;
    ei->rsp++;
    break;
  case 'r':
    PUSH = SET_REAL(rand() / (double)RAND_MAX);
    break;
  case 's':
    *ei->rsp = *(ei->rsp - (int)ei->rsp->elem.real - 1);
    break;
  }
}

static real_t handle_function_args(evalinfo_t *ei) {
  int argnum = *ei->expr - '0';
  if (ei->max_argc[ei->max_argci] < argnum)
    ei->max_argc[ei->max_argci] = argnum;
  return ei->argv[8 - argnum];
}

static void rpx_lvars(evalinfo_t *ei) {
  *++ei->rsp = (isdigit(*++ei->expr)) ? handle_function_args(ei)
               : (islower(*ei->expr)) ? ei->info.usrvar[*ei->expr - 'a']
                                      : *(real_t *)$panic(ERR_CHAR_NOT_FOUND);
}

static void rpx_wvars(evalinfo_t *ei) {
  ei->info.usrvar[*++ei->expr - 'a'] = *ei->rsp;
}

static void rpx_end(evalinfo_t *ei) { ei->iscontinue = false; }

static void rpx_grpbgn(evalinfo_t *ei) {
  (++ei->rsp)->elem.lamb = (char *)ei->rbp;
  ei->rbp = ei->rsp;
}

static void rpx_grpend(evalinfo_t *ei) {
  real_t *rbp = ei->rbp;
  ei->rbp = *(real_t **)ei->rbp;
  *rbp = *ei->rsp;
  ei->rsp = rbp;
}

static void rpx_lmdbgn(evalinfo_t *ei) {
  ei->expr++;
  int i = 0;
  int nest = 1;
  for (; *ei->expr; i++)
    if (ei->expr[i] == '{')
      nest++;
    else if (ei->expr[i] == '}' && !--nest)
      break;

  *++ei->rsp = SET_LAMB(malloc(i + 1));
  memcpy(ei->rsp->elem.lamb, ei->expr, i);
  ei->expr += i;
}

static void rpx_lmdend(evalinfo_t *ei) { _ = ei; }

static void call_fn(evalinfo_t *ei) {
  ei->callstack[++ei->callstacki] = ei->argv;
  ei->argv = ei->rsp - 8;
  ei->max_argc[++ei->max_argci] = 0;
  rpx_grpbgn(ei);
}

static void ret_fn(evalinfo_t *ei) {
  rpx_grpend(ei);
  real_t ret = *ei->rsp;
  ei->rsp = ei->argv + 8;
  ei->rsp -= ei->max_argc[ei->max_argci];
  *ei->rsp = ret;
  ei->argv = ei->callstack[ei->callstacki--];
}

static void rpx_runlmd(evalinfo_t *ei) {
  char const *temp = ei->expr;
  _ drop = ei->expr = ei->rsp->elem.lamb;
  call_fn(ei);
  rpx_eval(ei);
  ret_fn(ei);
  ei->expr = temp;
}

static void rpx_cond(evalinfo_t *ei) {
  ei->rsp -= 2 - !isnan(ei->rsp->elem.real);
}

static void rpx_undfned(evalinfo_t *ei) {
  disperr(__FUNCTION__, "%s: %c", codetomsg(ERR_UNKNOWN_CHAR), *ei->expr);
}

void (*eval_table['~' - ' ' + 1])(evalinfo_t *) = {
    rpx_space,   // ' '
    rpx_runlmd,  // '!'
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
    rpx_cond,    // '?'
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

void rpx_eval(evalinfo_t *ei) {
  for (; likely(*ei->expr && ei->iscontinue); ei->expr++)
    get_eval_table (*ei->expr)(ei);
}

evalinfo_t init_evalinfo() {
  evalinfo_t ret;
  ret.rbp = ret.rsp = ret.stack - 1;
  ret.info = get_rrtinfo();
  ret.iscontinue = true;
  ret.callstacki = ~0;
  return ret;
}

/**
 * @brief Evaluate real number expression
 * @param a_expr String of expression
 * @return Expression evaluation result
 */
elem_t eval_expr_real(char const *a_expr) {
  evalinfo_t ei = init_evalinfo();
  ei.expr = a_expr;
  rpx_eval(&ei);
  if (ei.info.histi < BUFSIZE)
    ei.info.hist[ei.info.histi++] = *ei.rsp;
  set_rrtinfo(ei.info);
  return (elem_t){{ei.rsp->elem.real}, ei.rsp->isnum ? RTYPE_REAL : RTYPE_LAMB};
}

test(eval_expr_real) {
  expecteq(11.0, eval_expr_real("5 6 + &x").elem.real);
  expecteq(22.0, eval_expr_real("$x 2 *").elem.real);

  // function
  expecteq("$1$1+", eval_expr_real("{$1$1+}&f").elem.lamb);
  expecteq(10.0, eval_expr_real("5$f!").elem.real);

  // nest group
  expecteq(33.0, eval_expr_real("4 5 (5 6 (6 7 +) +) +").elem.real);

  // lambda
  expecteq(8.0, eval_expr_real("4 {$1 2 *}!").elem.real);
  expecteq(19.0,
           eval_expr_real("1 5 {$1 3 +}! {5 $1 * {$1 4 -}! {$1 2 /}! $2 +}!")
               .elem.real);
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
