/**
 * @file src/main.c
 * @brief Define eval_expr_complex, proc_cmds
 */

#include "main.h"
#include "benchmarking.h"
#include "editline.h"
#include "elemop.h"
#include "error.h"
#include "evalfn.h"
#include "exproriented.h"
#include "gene.h"
#include "graphplot.h"
#include "mathdef.h"
#include "optexpr.h"
#include "phyconst.h"
#include "rand.h"
#include "rc.h"
#include "testing.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>

auto eval_f = evalExprReal;
auto print_complex = printComplexComplex;

int main(int argc, char const **argv) {
  initPlotCfg();
  loadInitScript(nullptr);

  procAList(argc, argv);

  readerLoop(stdin);

  return 0;
}

//! @brief Display help message
void startupMsg() {
  puts("===========================================================");
  puts("     RPX - Reverse Polish notation calculator eXtended");
  puts("===========================================================");
  puts(" A powerful RPN calculator supporting multiple modes:");
  puts("  - Real numbers      (default)");
  puts("  - Complex numbers   (:tc to toggle)");
  puts("");
  puts(" Features:");
  puts("  - Register ($a to $z)");
  puts("  - Result history (@a, @h)");
  puts("  - Special constants (\\P for pi, \\E for e)");
  puts("  - Advanced functions (sin, cos, log, etc.)");
  puts("  - Matrix operations (in complex mode)");
  puts("");
  puts(" Usage:");
  puts("  - Enter RPN expressions directly");
  puts("  - Use ':' for commands (e.g., :tc for complex mode)");
  puts("  - Start with ':o' for expression optimization");
  puts("");
  puts(" Examples:");
  puts("  3 4 + 5 *     -> 35");
  puts("  2 \\P * s      -> 0 (approximately)");
  puts("  [2 2,2,][2 1,2,3,4,]*  -> [8,12] (matrix multiplication)");
  puts("");
  puts(" Press Ctrl+D to exit the program.");
  puts("===========================================================");
}

/**
 * @brief Process input
 * @param[in] input_buf input
 */
[[gnu::nonnull]] void procInput(char const *restrict input_buf) {
  if (*input_buf == ':') {
    procCmds(input_buf + 1);
    return;
  }
  elem_t res;
  res = eval_f(input_buf);
  printElem(res);
}

/**
 * @brief Process argument list
 * @param[in] argc arg count
 * @param[in] argv arg value
 */
void procAList(int argc, char const **argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') { // interpreted as a file name
      FILE *fp dropfile
        = fopen(argv[i], "r") ?: p$panic(ERR_FILE_NOT_FOUND, "%s ", argv[i]);
      readerLoop(fp);
      continue;
    }

    switch (argv[i][1]) { // interpreted as a option
    case 'h':
      startupMsg();
      break;
    case 'r':
      procInput(argv[++i]);
      break;
    case 'q':
      exit(0);
    default:
      panic(ERR_UNKNOWN_OPTION, "%c ", argv[i][1]);
    }
  }
}

/**
 * @brief Read raw line from fp
 * @return is reached EOF
 */
bool readRawLine(char *buf, size_t len, FILE *fp) {
  return fgets(buf, (int)len, fp) != nullptr;
}

/**
 * @brief Read from stdin
 * @return is pressed ctrl_d
 */
bool readerInteractiveLine(char *buf, size_t len, FILE *fp) {
  _ = fp;
  return editline((int)len, buf);
}

/**
 * @brief File reading loop
 * @param[in] fp File stream
 */
[[gnu::nonnull]] void readerLoop(FILE *restrict fp) {
  char input_buf[buf_size];
  auto reader_fn = fp == stdin ? readerInteractiveLine : readRawLine;
  while (reader_fn(input_buf, buf_size, fp)) [[clang::likely]]
    procInput(input_buf);
}

/**
 * @brief Evaluate complex number expression
 * @param[in] expr String of expression
 * @return elem_t Expression evaluation result
 * @warning Possible stack overflow with very long expressions
 */
[[gnu::nonnull]] elem_t evalExprComplex(char const *expr) {
  elem_t operand_stack[buf_size] = {0};
  elem_t *rsp = operand_stack, *rbp = operand_stack;
  rtinfo_t info_c = getRuntimeInfo();

  for (;; expr++) {
    if (*expr == '[') {
      (++rsp)->rtype = RTYPE_MATR;
      expr++;
      matrix_t val = {.matrix = zalloc(complex, mat_init_size)};
      matrix curelem = val.matrix;
      val.cols = (size_t)strtol(expr, (char **)&expr, 10);
      for (; *expr != ']';) {
        *curelem++ = evalExprComplex(expr).elem.comp;
        skipUntilComma(&expr);
      }
      val.rows = (size_t)(curelem - val.matrix) / val.cols;
      rsp->elem.matr = val;
      continue;
    }
    if (isspace(*expr)) continue;
    if (*expr == '\0') break;

    switch (*expr) {
    case '0' ... '9':
      (++rsp)->rtype = RTYPE_COMP;
      rsp->elem.comp = strtod(expr, (char **)&expr);
      expr--;
      break;
    case '(':
      *(long *)&(++rsp)->elem.real = rbp - operand_stack;
      rbp = rsp;
      break;
    case ')':
      rbp = operand_stack + *(long *)&rbp->elem.real;
      rsp--;
      *rsp = *(rsp + 1);
      break;

      OP_CASE_ELEM(Add, +)
      OP_CASE_ELEM(Sub, -)
      OP_CASE_ELEM(Mul, *)
      OP_CASE_ELEM(Div, /)
      OP_CASE_ELEM(Pow, ^)
      OVERWRITE_COMP('A', fabs)
      OVERWRITE_COMP('s', sin)
      OVERWRITE_COMP('c', cos)
      OVERWRITE_COMP('t', tan)

    case '~': {
      _ drop = rsp->elem.matr.matrix;
      rsp->elem.matr = inverseMatrix(&rsp->elem.matr);
    } break;
    case '=':
      for (; rbp + 1 < rsp && elemEq(rsp - 1, rsp); rsp--);
      (rbp + 1)->elem.real = rbp + 1 == rsp;
      if (rbp + 1 != rsp) rsp = rbp + 1;
      break;

    case 'a':
      switch (*++expr) {
        OVERWRITE_COMP('s', asin)
        OVERWRITE_COMP('c', acos)
        OVERWRITE_COMP('t', atan)
      default:
        dispErr(__FUNCTION__, "unknown fn: %c", *expr);
      }
      break;

    case 'h':
      switch (*++expr) {
        OVERWRITE_COMP('s', sinh)
        OVERWRITE_COMP('c', cosh)
        OVERWRITE_COMP('t', tanh)
      default:
        dispErr(__FUNCTION__, "unknown fn: %c", *expr);
      }
      break;

    case 'l':
      rsp->elem.comp = clog(rsp->elem.comp);
      break;

    case 'L': { // log with base
      double x = creal((rsp--)->elem.comp);
      rsp->elem.comp = log(rsp->elem.comp) / log(x);
    } break;
    case 'r': // to radian
      rsp->elem.comp *= pi / 180;
      break;
    case 'd': // to degree
      rsp->elem.comp *= 180 / pi;
      break;
    case 'm': // negative sign
      rsp->elem.comp *= -1;
      break;
    case 'i': // imaginary number
      rsp->elem.comp *= I;
      break;
    case 'p': { // polar
      complex theta = (rsp--)->elem.comp;
      rsp->elem.comp
        = rsp->elem.comp * cos(theta) + I * rsp->elem.comp * sin(theta);
    } break;

    case '@':   // system functions
      switch (*++expr) {
      case 'a': // ANS
        elemSet(++rsp, &info_c.hist[info_c.histi]);
        break;
      case 'd':
        print_complex(rsp->elem.comp);
        break;
      case 'h': // history operation
        elemSet(rsp, &info_c.hist[info_c.histi - (size_t)rsp->elem.real]);
        break;
      case 'n':
        elemSet(++rsp, &(elem_t){.rtype = RTYPE_COMP, .elem = {.comp = NAN}});
        break;
      case 'p': // prev stack value
        rsp++;
        elemSet(rsp, rsp - 1);
        break;
      case 'r':
        (++rsp)->elem.comp = xorsh0to1();
        break;
      case 's': // stack value operation
        elemSet(rsp, rsp - (int)rsp->elem.real - 1);
        break;
      default:
        [[clang::unlikely]];
      }
      break;

    case '\\': // special variables and CONSTANTS
      (++rsp)->elem.comp = getConst(*++expr);
      break;

    case '$': // register
      if (islower(*++expr)) [[clang::likely]] {
        elem_t *rhs = &info_c.reg[*expr - 'a'];
        elemSet(++rsp, rhs);
      }
      break;

    case '&':
      elemSet(&info_c.reg[*++expr - 'a'], rsp);
      break;

      // TODO differential
      // TODO integral
      // TODO sigma

    case ';': // comment
    case ',': // delimiter
      goto end;
    default:
      dispErr(__FUNCTION__, "unknown char: %c", *expr);
    }
  }

end:
  if (rsp->rtype == RTYPE_MATR) {
    elem_t *rhs = &info_c.hist[++info_c.histi];
    if (rhs->rtype == RTYPE_MATR) nfree(rhs->elem.matr.matrix);
    *rhs = *rsp;
  } else if (info_c.histi < buf_size)
    info_c.hist[++info_c.histi].elem.comp = rsp->elem.comp;
  setRuntimeInfo(info_c);
  return *rsp;
}

#define eval_expr_complex_return_complex(expr) evalExprComplex(expr).elem.comp
test_table(
  eval_complex, eval_expr_complex_return_complex, (complex, char const *),
  {
    {1.0 + 2.0i,        "1 2i +"},
    {   1024.0i,        "4i 5 ^"},
    {      4.0i, "1 1i+(2 2i+)*"},
}
)
test_table(
  eval_complex_comp, eval_expr_complex_return_complex, (complex, char const *),
  {
    {1.2984575814159773 + 0.6349639147847361i, "1 1i+s"},
    {1.1447298858494002 + 1.5707963267948967i,  "\\Pil"},
}
)
#undef eval_expr_complex_return_complex

test (eval_expr_complex) {
  matrix_t resultm;

  // Test matrix addition
  resultm = evalExprComplex("[2 1,2,3,4,][2 5,6,7,8,]+").elem.matr;
  expecteq(6.0, resultm.matrix[0]);
  expecteq(8.0, resultm.matrix[1]);
  expecteq(10.0, resultm.matrix[2]);
  expecteq(12.0, resultm.matrix[3]);

  // Test matrix multiplication
  char const *expr = "[2 1,2,3,4,][2 5,6,7,8,]*";
  resultm = evalExprComplex(expr).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(19.0, resultm.matrix[0]);
  expecteq(22.0, resultm.matrix[1]);
  expecteq(43.0, resultm.matrix[2]);
  expecteq(50.0, resultm.matrix[3]);

  // Test matrix inverse
  expr = "[2 1,2,3,4,]~";
  resultm = evalExprComplex(expr).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(-2.0, resultm.matrix[0]);
  expecteq(1.0, resultm.matrix[1]);
  expecteq(1.5, resultm.matrix[2]);
  expecteq(-0.5, resultm.matrix[3]);

  // Scalar multiplication
  expr = "[3 5,6,7,] 5 *";
  resultm = evalExprComplex(expr).elem.matr;
  expecteq(1, resultm.rows);
  expecteq(3, resultm.cols);
  expecteq(25, resultm.matrix[0]);
  expecteq(30, resultm.matrix[1]);
  expecteq(35, resultm.matrix[2]);
}

bench (eval_expr_complex) {
  evalExprComplex("1 2 3 4 5 +");
  evalExprComplex("4 5 ^");
  evalExprComplex("1s2^(1c2^)+");
  evalExprComplex("  5    6    10    - 5  /");
  evalExprComplex("5");
  evalExprComplex("@a");
  evalExprComplex("10 &x");
  evalExprComplex("$x 2 *");
  evalExprComplex("2 3 ^ (4 5 *) + (6 7 /) -");
  evalExprComplex("\\P 2 / s");
  evalExprComplex("\\P 4 / c");
  evalExprComplex("2 l2");
  evalExprComplex("100 lc");
  evalExprComplex("1 0 /");
}

bench (eval_matrix) {
  evalExprComplex("[2 1,2,3,4,][2 5,6,7,8,]+");
  evalExprComplex("[3 4,1,4,6,5,7,3,6,7,]~");
  evalExprComplex("[1 4,5,][2 6,7,]*");
  evalExprComplex("[2 7,6,5,4,] 6 *");
  evalExprComplex("[2 9,0,5,1,] 4 ^");
  evalExprComplex("[2 5,4,3,2,][2 4,8,2,1,]/");
}

/**
 * @brief Output elem_t in appropriate format
 * @param[in] elem Output comtent
 */
void printElem(elem_t elem) {
  switch (elem.rtype) {
  case RTYPE_REAL:
    printReal(elem.elem.real);
    break;
  case RTYPE_COMP:
    print_complex(elem.elem.comp);
    break;
  case RTYPE_MATR:
    printMatrix(elem.elem.matr);
    free(elem.elem.matr.matrix);
    break;
  case RTYPE_LAMB:
    printLambda(elem.elem.lamb);
    free(elem.elem.lamb);
    break;
  default:
    [[clang::unlikely]];
  }
}

overloadable void printany(elem_t elem) {
  printElem(elem);
}

overloadable bool eq(elem_t lhs, elem_t rhs) {
  if (lhs.rtype != rhs.rtype) return false;
  switch (lhs.rtype) {
  case RTYPE_REAL:
    return eq(lhs.elem.real, rhs.elem.real);
  case RTYPE_COMP:
    return eq(lhs.elem.comp, rhs.elem.comp);
  case RTYPE_MATR:
    return eq(&lhs.elem.matr, &rhs.elem.matr);
  case RTYPE_LAMB:
    return eq(lhs.elem.lamb, rhs.elem.lamb);
  default:
    return false;
  }
}

/**
 * @brief Output value of type double
 * @param[in] result Output value
 */
void printReal(double result) {
  if (isInt(result)) PRINT("result: ", (long)result, "\n");
  else PRINT("result: ", result, "\n");
}

/**
 * @brief Output value of type complex
 */
void printComplexComplex(complex result) {
  PRINT("result: ", creal(result), " + ", cimag(result), "i\n");
}

/**
 * @brief Output value of type complex in phasor view
 */
void printComplexPolar(complex result) {
  complex res = result;
  if (isnan(creal(res)) || isnan(cimag(res))) return;

  PRINT(
    "result: ", cabs(res), " \\phasor ", atan2(cimag(res), creal(res)), "\n"
  );
}

/**
 * @brief Output value of type matrix_t
 */
void printMatrix(matrix_t result) {
  for (size_t i = 0; i < result.rows; i++) {
    for (size_t j = 0; j < result.cols; j++) {
      complex res = result.matrix[result.cols * i + j];
      putchar('\t');
      if (cimag(res) == 0) PRINT(creal(res));
      else PRINT(creal(res), " + ", cimag(res), "i");
    }

    putchar('\n');
  }
}

[[gnu::nonnull]] void printLambda(char const *result) {
  PRINT("result: ", result, "\n");
}

/**
 * @brief Process command
 * @param[in] cmd Command input
 */
[[gnu::nonnull]] void procCmds(char const *restrict cmd) {
  plotcfg_t pcfg = getPlotCfg();

  // TODO save registers
  switch (*cmd++) {
  case 't':   // toggle
    switch (*cmd) {
    case 'c': // complex mode
      eval_f = eval_f == evalExprReal ? evalExprComplex : evalExprReal;
      break;
    case 'p': // plotcfg explicit implicit
      pcfg.plotExpr = pcfg.plotExpr == plotexpr ? plotexprImplicit : plotexpr;
      setPlotCfg(pcfg);
      break;
    case 'P': // print_complex
      print_complex = print_complex == printComplexComplex
                      ? printComplexPolar
                      : printComplexComplex;
      break;
    default:
      [[clang::unlikely]];
    }
    break;
  case 'o': {
    char buf[buf_size];
    strncpy(buf, cmd, buf_size);
    optexpr(buf);
    puts(buf);
  } break;
  case 'p':
    skipSpaces((char const **)&cmd);
    if (*cmd == '\0') {
      plotexpr(pcfg.prevexpr);
      break;
    }

    pcfg.plotExpr(cmd);
    strncpy(pcfg.prevexpr, cmd, buf_size);
    setPlotCfg(pcfg);
    break;
  case 'r':   // rand
    switch (*cmd) {
    case 's': // set seed
      sxorsh((uint64_t)evalExprReal(cmd + 1).elem.real);
      break;
    default:
    }
    break;
  case 's':   // settings
    switch (*cmd) {
    case 'p': // plot
      changePlotCfg(cmd + 1);
      break;
    default:
      [[clang::unlikely]];
    }
    break;
  default:
    dispErr(__FUNCTION__, "unknown command: %c", *(cmd - 1));
  }
}
