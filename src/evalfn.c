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

elem_t evalExprReal(char const *);

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
  static void rpx##tok(machine_t *ei) { \
    for (; ei->s.rbp + 1 < ei->s.rsp; \
         ei->s.rbp[1].elem.real op## = POP.elem.real); \
  }
APPLY_ARTHM(DEF_ARTHMS)

static void rpxMod(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp;
       ei->s.rbp[1].elem.real = fmod(ei->s.rbp[1].elem.real, POP.elem.real));
}

static void rpxPow(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp;
       ei->s.rbp[1].elem.real = pow(ei->s.rbp[1].elem.real, POP.elem.real));
}

static void rpxEql(machine_t *ei) {
  for (; ei->s.rbp + 1 < ei->s.rsp
         && eq(ei->s.rsp[-1].elem.real, ei->s.rsp->elem.real);
       POP);
  ei->s.rbp[1].elem.real = ei->s.rbp + 1 == ei->s.rsp ?: NAN;
  ei->s.rsp = ei->s.rbp + 1;
}

#define DEF_LTGT(tok, op) \
  static void rpx##tok(machine_t *ei) { \
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
DEF_MULTI(torad, pi / 180)
DEF_MULTI(todeg, 180 / pi)

#define DEF_TWOCHARFN(name, c1, f1, c2, f2, c3, f3) \
  static void rpx_##name(machine_t *ei) { \
    switch (*++ei->c.rip) { \
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

static void rpxLogBase(machine_t *ei) {
  double x = POP.elem.real;
  ei->s.rsp->elem.real = log(ei->s.rsp->elem.real) / log(x);
}

static void rpxConst(machine_t *ei) {
  PUSH = SET_REAL(getConst(*++ei->c.rip));
}

static void rpxParse(machine_t *ei) {
  char *next = nullptr;
  PUSH = SET_REAL(strtod(ei->c.rip, &next));
  ei->c.rip = next - 1;
}

static void rpxSpace(machine_t *ei) {
  skipSpaces(&ei->c.rip);
  ei->c.rip--;
}

#define CASE_TWOARGFN(c, f) \
  case c: { \
    double x = POP.elem.real; \
    ei->s.rsp->elem.real = f(ei->s.rsp->elem.real, x); \
  } break;
static void rpxIntFn(machine_t *ei) {
  switch (*++ei->c.rip) {
    CASE_TWOARGFN('g', gcd)
    CASE_TWOARGFN('l', lcm)
    CASE_TWOARGFN('p', permutation)
    CASE_TWOARGFN('c', combination)
  default:
    [[clang::unlikely]];
  }
}

static void rpxSysFn(machine_t *ei) {
  switch (*++ei->c.rip) {
  case 'a': // ANS
    PUSH = ei->e.info.hist[lesser(ei->e.info.histi, buf_size - 1)];
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
    PUSH = SET_REAL(xorsh0to1());
    break;
  case 's':
    *ei->s.rsp = *(ei->s.rsp - (int)ei->s.rsp->elem.real - 1);
    break;
  default:
    [[clang::unlikely]];
  }
}

static real_t handleFnArgs(machine_t *ei) {
  char argnum = *ei->c.rip - '0';
  if (ei->d.argc[ei->d.argci] < argnum) ei->d.argc[ei->d.argci] = argnum;
  return ei->e.args[8 - argnum];
}

static void rpxLRegs(machine_t *ei) {
  *++ei->s.rsp = (isdigit(*++ei->c.rip)) ? handleFnArgs(ei)
               : (islower(*ei->c.rip))   ? ei->e.info.reg[*ei->c.rip - 'a']
                                       : *(real_t *)$panic(ERR_CHAR_NOT_FOUND);
}

static void rpxWRegs(machine_t *ei) {
  ei->e.info.reg[*++ei->c.rip - 'a'] = *ei->s.rsp;
}

static void rpxEnd(machine_t *ei) {
  ei->e.iscontinue = false;
}

static void rpxGrpBgn(machine_t *ei) {
  PUSH.elem.lamb = (char *)ei->s.rbp;
  ei->s.rbp = ei->s.rsp;
}

static void rpxGrpEnd(machine_t *ei) {
  real_t *rbp = ei->s.rbp;
  ei->s.rbp = *(real_t **)ei->s.rbp;
  *rbp = *ei->s.rsp;
  ei->s.rsp = rbp;
}

static void rpxLmdBgn(machine_t *ei) {
  ei->c.rip++;
  size_t i = 0;
  for (int nest = 1; *ei->c.rip; i++)
    if (ei->c.rip[i] == '{') nest++;
    else if (ei->c.rip[i] == '}' && !--nest) break;

  *++ei->s.rsp = SET_LAMB(zalloc(char, i + 1));
  memcpy(ei->s.rsp->elem.lamb, ei->c.rip, i);
  ei->s.rsp->elem.lamb[i] = '\0';
  ei->c.rip += i;
}

static void rpxLmbEnd(machine_t *ei) {
  _ = ei;
}

static void callFn(machine_t *ei) {
  ei->d.callstack[++ei->d.callstacki] = ei->e.args;
  ei->e.args = ei->s.rsp - 8;
  ei->d.argc[++ei->d.argci] = 0;
  rpxGrpBgn(ei);
}

static void retFn(machine_t *ei) {
  rpxGrpEnd(ei);
  real_t ret = *ei->s.rsp;
  ei->s.rsp = ei->e.args + 8;
  ei->s.rsp -= ei->d.argc[ei->d.argci--];
  *ei->s.rsp = ret;
  ei->e.args = ei->d.callstack[ei->d.callstacki--];
}

static void rpxRunLmd(machine_t *ei) {
  char const *temp = ei->c.rip;
  _ drop = ei->c.expr = ei->c.rip = ei->s.rsp->elem.lamb;
  callFn(ei);
  rpxEval(ei);
  retFn(ei);
  ei->c.rip = temp;
}

static void rpxCond(machine_t *ei) {
  ei->s.rsp -= 2;
  real_t *rsp = ei->s.rsp;
  *rsp = *(rsp + isnan(rsp[2].elem.real));
}

static void rpxUndfned(machine_t *ei) {
  dispErr(
    __FUNCTION__,
    "%s: %c at col %zu",
    codetomsg(ERR_UNKNOWN_CHAR),
    *ei->c.rip,
    ei->c.rip - ei->c.expr
  );
}

void (*const eval_table['~' - ' ' + 1])(machine_t *) = {
  rpxSpace,   // ' '
  rpxRunLmd,  // '!'
  rpxUndfned, // '"'
  rpxUndfned, // '#'
  rpxLRegs,   // '$'
  rpxMod,     // '%'
  rpxWRegs,   // '&'
  rpxUndfned, // '''
  rpxGrpBgn,  // '('
  rpxGrpEnd,  // ')'
  rpxMul,     // '*'
  rpxAdd,     // '+'
  rpxEnd,     // ','
  rpxSub,     // '-'
  rpxUndfned, // '.'
  rpxDiv,     // '/'
  rpxParse,   // '0'
  rpxParse,   // '1'
  rpxParse,   // '2'
  rpxParse,   // '3'
  rpxParse,   // '4'
  rpxParse,   // '5'
  rpxParse,   // '6'
  rpxParse,   // '7'
  rpxParse,   // '8'
  rpxParse,   // '9'
  rpxUndfned, // ':'
  rpxEnd,     // ';'
  rpxLt,      // '<'
  rpxEql,     // '='
  rpxGt,      // '>'
  rpxCond,    // '?'
  rpxSysFn,   // '@'
  rpx_fabs,   // 'A'
  rpxUndfned, // 'B'
  rpx_ceil,   // 'C'
  rpxUndfned, // 'D'
  rpxUndfned, // 'E'
  rpx_floor,  // 'F'
  rpxUndfned, // 'G'
  rpxUndfned, // 'H'
  rpxUndfned, // 'I'
  rpxUndfned, // 'J'
  rpxUndfned, // 'K'
  rpxLogBase, // 'L'
  rpxUndfned, // 'M'
  rpxUndfned, // 'N'
  rpxUndfned, // 'O'
  rpxUndfned, // 'P'
  rpxUndfned, // 'Q'
  rpx_round,  // 'R'
  rpxUndfned, // 'S'
  rpxUndfned, // 'T'
  rpxUndfned, // 'U'
  rpxUndfned, // 'V'
  rpxUndfned, // 'W'
  rpxUndfned, // 'X'
  rpxUndfned, // 'Y'
  rpxUndfned, // 'Z'
  rpxUndfned, // '['
  rpxConst,   // '\'
  rpxUndfned, // ']'
  rpxPow,     // '^'
  rpxUndfned, // '_'
  rpxUndfned, // '`'
  rpx_arc,    // 'a'
  rpxUndfned, // 'b'
  rpx_cos,    // 'c'
  rpx_todeg,  // 'd'
  rpxUndfned, // 'e'
  rpxUndfned, // 'f'
  rpx_tgamma, // 'g'
  rpx_hyp,    // 'h'
  rpxIntFn,   // 'i'
  rpxUndfned, // 'j'
  rpxUndfned, // 'k'
  rpx_log,    // 'l'
  rpx_negate, // 'm'
  rpxUndfned, // 'n'
  rpxUndfned, // 'o'
  rpxUndfned, // 'p'
  rpxUndfned, // 'q'
  rpx_torad,  // 'r'
  rpx_sin,    // 's'
  rpx_tan,    // 't'
  rpxUndfned, // 'u'
  rpxUndfned, // 'v'
  rpxUndfned, // 'w'
  rpxUndfned, // 'x'
  rpxUndfned, // 'y'
  rpxUndfned, // 'z'
  rpxLmdBgn,  // '{'
  rpxUndfned, // '|'
  rpxLmbEnd,  // '}'
  rpxUndfned, // '~'
};

void (*getEvalTable(char c))(machine_t *) {
  return eval_table[c - ' '];
}

void rpxEval(machine_t *restrict ei) {
  for (; *ei->c.rip && ei->e.iscontinue; ei->c.rip++) [[clang::likely]]
    getEvalTable (*ei->c.rip)(ei);
}

void initEvalinfo(machine_t *restrict ret) {
  ret->s.rbp = ret->s.rsp = ret->s.payload;
  ret->e.info = getRRuntimeInfo();
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
elem_t evalExprReal(char const *restrict a_expr) {
  machine_t ei;
  initEvalinfo(&ei);
  ei.c.expr = ei.c.rip = a_expr;
  rpxEval(&ei);
  if (++ei.e.info.histi < buf_size) ei.e.info.hist[ei.e.info.histi] = *ei.s.rsp;
  setRRuntimeInfo(ei.e.info);
  return (elem_t){{ei.s.rsp->elem.real},
                  ei.s.rsp->isnum ? RTYPE_REAL : RTYPE_LAMB};
}

test (eval_expr_real) {
  // function
  expecteq("$1$1+", evalExprReal("{$1$1+}&f").elem.lamb);
  expecteq(10.0, evalExprReal("5$f!").elem.real);
}

#define eval_expr_real_return_double(expr) evalExprReal(expr).elem.real
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
  evalExprReal("1 2 3 4 5 +");
  evalExprReal("4 5 ^");
  evalExprReal("1s2^(1c2^)+");
  evalExprReal("  5    6    10    - 5  /");
  evalExprReal("5");
  evalExprReal("@a");
  evalExprReal("10 &x");
  evalExprReal("$x 2 *");
  evalExprReal("2 3 ^ (4 5 *) + (6 7 /) -");
  evalExprReal("\\P 2 / s");
  evalExprReal("\\P 4 / c");
  evalExprReal("2 l2");
  evalExprReal("100 lc");
  evalExprReal("1 0 /");
}
