/**
 * @file src/evalfn.c
 * @brief Define eval_expr_real related functions
 */

#include "evalfn.h"
#include "arthfn.h"
#include "benchmarking.h"
#include "error.h"
#include "exproriented.h"
#include "gene.h"
#include "mathdef.h"
#include "phyconst.h"
#include "rand.h"
#include "testing.h"
#include <ctype.h>
#include <string.h>

elem_t eval_expr_real(char const *);

#define PUSH (*++ei->s.rsp)
#define POP  (*ei->s.rsp--)

#define SET_REAL(v) \
  (real_t) { \
    .elem = {.real = v}, .isnum = true \
  }
#define SET_LAMB(v) \
  (real_t) { \
    .elem = {.lamb = v}, .isnum = false \
  }

#define DEF_ARTHMS(tok, op) \
  static void rpx_##tok(machine_t *ei) { \
    for (; ei->s.rbp + 1 < ei->s.rsp; \
         ei->s.rbp[1].elem.real op## = POP.elem.real); \
  }
APPLY_ARTHM(DEF_ARTHMS)

static void rpx_mod(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp;
       ei->s.rbp[1].elem.real = fmod(ei->s.rbp[1].elem.real, POP.elem.real));
}

static void rpx_pow(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp;
       ei->s.rbp[1].elem.real = pow(ei->s.rbp[1].elem.real, POP.elem.real));
}

static void rpx_eql(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp
         && eq(ei->s.rsp[-1].elem.real, ei->s.rsp->elem.real);
       POP);
  ei->s.rbp[1].elem.real = ei->s.rbp + 1 == ei->s.rsp ?: NAN;
  ei->s.rsp = ei->s.rbp + 1;
}

#define DEF_LTGT(tok, op) \
  static void rpx_##tok(machine_t *ei) { \
    for (; ei->s.rbp + 1 < ei->s.rsp \
           && ei->s.rsp[-1].elem.real op ei->s.rsp->elem.real; \
         POP); \
    ei->s.rbp[1].elem.real = ei->s.rbp + 1 == ei->s.rsp ?: NAN; \
    ei->s.rsp = ei->s.rbp + 1; \
  }
APPLY_LTGT(DEF_LTGT)

#define DEF_ONEARGFN(f) \
  static void rpx_##f(machine_t *ei) { \
    ei->s.rsp->elem.real = f(ei->s.rsp->elem.real); \
  }
DEF_ONEARGFN(sin)
DEF_ONEARGFN(cos)
DEF_ONEARGFN(tan)
DEF_ONEARGFN(fabs)
DEF_ONEARGFN(tgamma)
DEF_ONEARGFN(ceil)
DEF_ONEARGFN(floor)
DEF_ONEARGFN(round)

#define DEF_MULTI(name, factor) \
  static void rpx_##name(machine_t *ei) { \
    ei->s.rsp->elem.real *= factor; \
  }
DEF_MULTI(negate, -1)
DEF_MULTI(torad, M_PI / 180)
DEF_MULTI(todeg, 180 / M_PI)

#define DEF_TWOCHARFN(name, c1, f1, c2, f2, c3, f3) \
  static void rpx_##name(machine_t *ei) { \
    switch (*++ei->c.expr) { \
      OVERWRITE_REAL(c1, f1) \
      OVERWRITE_REAL(c2, f2) \
      OVERWRITE_REAL(c3, f3) \
    default: \
      [[clang::unlikely]]; \
    } \
  }
DEF_TWOCHARFN(hyp, 's', sinh, 'c', cosh, 't', tanh)
DEF_TWOCHARFN(arc, 's', asin, 'c', acos, 't', atan)
DEF_TWOCHARFN(log, '2', log2, 'c', log10, 'e', log)

static void rpx_logbase(machine_t *ei) {
  double x = POP.elem.real;
  ei->s.rsp->elem.real = log(ei->s.rsp->elem.real) / log(x);
}

static void rpx_const(machine_t *ei) {
  PUSH = SET_REAL(get_const(*++ei->c.expr));
}

static void rpx_parse(machine_t *ei) {
  char *next = nullptr;
  PUSH = SET_REAL(strtod(ei->c.expr, &next));
  ei->c.expr = next - 1;
}

static void rpx_space(machine_t *ei) {
  skipspcs(&ei->c.expr);
  ei->c.expr--;
}

#define CASE_TWOARGFN(c, f) \
  case c: { \
    double x = POP.elem.real; \
    ei->s.rsp->elem.real = f(ei->s.rsp->elem.real, x); \
  } break;
static void rpx_intfn(machine_t *ei) {
  switch (*++ei->c.expr) {
    CASE_TWOARGFN('g', gcd)
    CASE_TWOARGFN('l', lcm)
    CASE_TWOARGFN('p', permutation)
    CASE_TWOARGFN('c', combination)
  default:
    [[clang::unlikely]];
  }
}

static void rpx_sysfn(machine_t *ei) {
  switch (*++ei->c.expr) {
  case 'a': // ANS
    PUSH = ei->e.info.hist[lesser(ei->e.info.histi, BUFSIZE - 1)];
    break;
  case 'd': // display
    printany(ei->s.rsp->elem.real);
    putchar('\n');
    break;
  case 'h':
    ei->s.rsp->elem.real
      = ei->e.info.hist[ei->e.info.histi - (size_t)ei->s.rsp->elem.real]
          .elem.real;
    break;
  case 'n':
    PUSH = SET_REAL(NAN);
    break;
  case 'p':
    ei->s.rsp[1] = *ei->s.rsp;
    ei->s.rsp++;
    break;
  case 'r':
    PUSH = SET_REAL(xorsh_0_1());
    break;
  case 's':
    *ei->s.rsp = *(ei->s.rsp - (int)ei->s.rsp->elem.real - 1);
    break;
  default:
    [[clang::unlikely]];
  }
}

static real_t handle_function_args(machine_t *ei) {
  char argnum = *ei->c.expr - '0';
  if (ei->d.argc[ei->d.argci] < argnum) ei->d.argc[ei->d.argci] = argnum;
  return ei->e.args[8 - argnum];
}

static void rpx_lregs(machine_t *ei) {
  *++ei->s.rsp = (isdigit(*++ei->c.expr)) ? handle_function_args(ei)
               : (islower(*ei->c.expr))   ? ei->e.info.reg[*ei->c.expr - 'a']
                                        : *(real_t *)$panic(ERR_CHAR_NOT_FOUND);
}

static void rpx_wregs(machine_t *ei) {
  ei->e.info.reg[*++ei->c.expr - 'a'] = *ei->s.rsp;
}

static void rpx_end(machine_t *ei) {
  ei->e.iscontinue = false;
}

static void rpx_grpbgn(machine_t *ei) {
  PUSH.elem.lamb = (char *)ei->s.rbp;
  ei->s.rbp = ei->s.rsp;
}

static void rpx_grpend(machine_t *ei) {
  real_t *rbp = ei->s.rbp;
  ei->s.rbp = *(real_t **)ei->s.rbp;
  *rbp = *ei->s.rsp;
  ei->s.rsp = rbp;
}

static void rpx_lmdbgn(machine_t *ei) {
  ei->c.expr++;
  size_t i = 0;
  for (int nest = 1; *ei->c.expr; i++)
    if (ei->c.expr[i] == '{') nest++;
    else if (ei->c.expr[i] == '}' && !--nest) break;

  *++ei->s.rsp = SET_LAMB(zalloc(char, i + 1));
  memcpy(ei->s.rsp->elem.lamb, ei->c.expr, i);
  ei->s.rsp->elem.lamb[i] = '\0';
  ei->c.expr += i;
}

static void rpx_lmdend(machine_t *ei) {
  _ = ei;
}

static void call_fn(machine_t *ei) {
  ei->d.callstack[++ei->d.callstacki] = ei->e.args;
  ei->e.args = ei->s.rsp - 8;
  ei->d.argc[++ei->d.argci] = 0;
  rpx_grpbgn(ei);
}

static void ret_fn(machine_t *ei) {
  rpx_grpend(ei);
  real_t ret = *ei->s.rsp;
  ei->s.rsp = ei->e.args + 8;
  ei->s.rsp -= ei->d.argc[ei->d.argci--];
  *ei->s.rsp = ret;
  ei->e.args = ei->d.callstack[ei->d.callstacki--];
}

static void rpx_runlmd(machine_t *ei) {
  char const *temp = ei->c.expr;
  _ drop = ei->c.expr = ei->s.rsp->elem.lamb;
  call_fn(ei);
  rpx_eval(ei);
  ret_fn(ei);
  ei->c.expr = temp;
}

static void rpx_cond(machine_t *ei) {
  ei->s.rsp -= 2;
  real_t *rsp = ei->s.rsp;
  *rsp = *(rsp + isnan(rsp[2].elem.real));
}

static void rpx_undfned(machine_t *ei) {
  disperr(__FUNCTION__, "%s: %c", codetomsg(ERR_UNKNOWN_CHAR), *ei->c.expr);
}

void (*const eval_table['~' - ' ' + 1])(machine_t *) = {
  rpx_space,   // ' '
  rpx_runlmd,  // '!'
  rpx_undfned, // '"'
  rpx_undfned, // '#'
  rpx_lregs,   // '$'
  rpx_mod,     // '%'
  rpx_wregs,   // '&'
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
  rpx_negate,  // 'm'
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

void (*get_eval_table(char c))(machine_t *) {
  return eval_table[c - ' '];
}

[[gnu::nonnull]] void rpx_eval(machine_t *restrict ei) {
  for (; *ei->c.expr && ei->e.iscontinue; ei->c.expr++) [[clang::likely]]
    get_eval_table (*ei->c.expr)(ei);
}

[[gnu::nonnull]] void init_evalinfo(machine_t *restrict ret) {
  ret->s.rbp = ret->s.rsp = ret->s.payload;
  ret->e.info = get_rrtinfo();
  ret->e.iscontinue = true;
  ret->d.argci = 0;
  ret->d.callstacki = ~(unsigned)0;
  memset(ret->d.argc, 0, sizeof ret->d.argc);
}

/**
 * @brief Evaluate real number expression
 * @param a_expr String of expression
 * @return Expression evaluation result
 */
[[gnu::nonnull]] elem_t eval_expr_real(char const *restrict a_expr) {
  machine_t ei;
  init_evalinfo(&ei);
  ei.c.expr = a_expr;
  rpx_eval(&ei);
  if (++ei.e.info.histi < BUFSIZE) ei.e.info.hist[ei.e.info.histi] = *ei.s.rsp;
  set_rrtinfo(ei.e.info);
  return (elem_t){{ei.s.rsp->elem.real},
                  ei.s.rsp->isnum ? RTYPE_REAL : RTYPE_LAMB};
}

test (eval_expr_real) {
  // function
  expecteq("$1$1+", eval_expr_real("{$1$1+}&f").elem.lamb);
  expecteq(10.0, eval_expr_real("5$f!").elem.real);
}

#define eval_expr_real_return_double(expr) eval_expr_real(expr).elem.real
test_table(
  eval_real, eval_expr_real_return_double, (double, char const *),
  {
    {11.0,              "5 6 + &x"}, // write reg
    {22.0,                "$x 2 *"}, // load reg
    {33.0, "4 5 (5 6 (6 7 +) +) +"}, // nest grp
    {66.0,               "@a @a +"}, // ans
    { 4.0,            "1 1 + @p +"}, // prev
    { 0.0,                 "\\P s"}, // const
}
)
test_table(
  eval_lamb, eval_expr_real_return_double, (double, char const *),
  {
    { 8.0,                                      "4 {$1 2 *}!"}, // lamb
    {19.0, "1 5 {$1 3 +}! {5 $1 * {$1 4 -}! {$1 2 /}! $2 +}!"}, // nest lamb
}
)
#undef eval_expr_real_return_double

bench (eval_expr_real) {
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
