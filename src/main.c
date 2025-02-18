//! @file main.c

/*
 * PUSH:      *++rsp = val;
 * POP:       val = *rsp--;
 * OVERWRITE: *rsp = val;
 * PREV BASE: val = *rbp;
 *
 * matrix_t:  double complex *
 * SNAN:      (double)nan
 * BUFSIZE:   0x100
 * rtype_t:   enum {RTYPE_REAL, RTYPE_COMP, RTYPE_MATR, RTYPE_LAST}
 * result_t:  union {double, double complex, matrix_t}
 */

#include "main.h"
#include "arthfn.h"
#include "benchmarking.h"
#include "editline.h"
#include "elemop.h"
#include "error.h"
#include "evalfn.h"
#include "exproriented.h"
#include "graphplot.h"
#include "optexpr.h"
#include "phyconst.h"
#include "rc.h"
#include "testing.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

auto eval_f = eval_expr_real;

int main(int argc, char const **argv) {
  srand((unsigned int)time(nullptr));

  init_plotconfig();
  load_initscript(nullptr);

  proc_alist(argc, argv);

  reader_loop(stdin);

  return 0;
}

//! @brief Display help message
void startup_message() {
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
[[gnu::nonnull]] void proc_input(char const *restrict input_buf) {
  if (*input_buf == ':') {
    proc_cmds(input_buf + 1);
    return;
  }
  elem_t res;
  res = eval_f(input_buf);
  print_elem(res);
}

/**
 * @brief Process argument list
 * @param[in] argc arg count
 * @param[in] argv arg value
 */
void proc_alist(int argc, char const **argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') { // interpreted as a file name
      FILE *fp dropfile =
          fopen(argv[i], "r") ?: p$panic(ERR_FILE_NOT_FOUND, "%s ", argv[i]);
      reader_loop(fp);
      continue;
    }

    switch (argv[i][1]) { // interpreted as a option
    case 'h':
      startup_message();
      break;
    case 'r':
      proc_input(argv[++i]);
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
bool read_raw_line(char *buf, size_t len, FILE *fp) {
  return fgets(buf, (int)len, fp) != nullptr;
}

/**
 * @brief Read from stdin
 * @return is pressed ctrl_d
 */
bool reader_interactive_line(char *buf, size_t len, FILE *fp) {
  _ = fp;
  return editline((int)len, buf);
}

/**
 * @brief File reading loop
 * @param[in] fp File stream
 */
[[gnu::nonnull]] void reader_loop(FILE *restrict fp) {
  char input_buf[BUFSIZE];
  auto reader_fn = fp == stdin ? reader_interactive_line : read_raw_line;
  while (reader_fn(input_buf, BUFSIZE, fp)) [[clang::likely]]
    proc_input(input_buf);
}

/**
 * @brief Evaluate complex number expression
 * @param expr String of expression
 * @return elem_t Expression evaluation result
 */
[[gnu::nonnull]] elem_t eval_expr_complex(char const *expr) {
  elem_t operand_stack[BUFSIZE] = {0};
  elem_t *rsp = operand_stack, *rbp = operand_stack;
  rtinfo_t info_c = get_rtinfo();

  for (;; expr++) {
    if (isdigit(*expr)) {
      (++rsp)->rtype = RTYPE_COMP;
      rsp->elem.comp = strtod(expr, (char **)&expr);
    }
    if (*expr == '[') {
      (++rsp)->rtype = RTYPE_MATR;
      expr++;
      matrix_t val = {.matrix = palloc(MAT_INITSIZE * sizeof(double complex))};
      matrix curelem = val.matrix;
      val.cols = (size_t)strtol(expr, (char **)&expr, 10);
      for (; *expr != ']';) {
        *curelem++ = eval_expr_complex(expr).elem.comp;
        skip_untilcomma(&expr);
      }
      val.rows = (size_t)(curelem - val.matrix) / val.cols;
      rsp->elem.matr = val;
      continue;
    }
    if (isspace(*expr))
      continue;
    if (*expr == '\0')
      break;

    switch (*expr) {
    case '(':
      *(long *)&(++rsp)->elem.real = rbp - operand_stack;
      rbp = rsp;
      break;
    case ')':
      rbp = operand_stack + *(long *)&rbp->elem.real;
      rsp--;
      *rsp = *(rsp + 1);
      break;

      OP_CASE_ELEM(add, +)
      OP_CASE_ELEM(sub, -)
      OP_CASE_ELEM(mul, *)
      OP_CASE_ELEM(div, /)
      OP_CASE_ELEM(pow, ^)
      OVERWRITE_COMP('A', fabs)
      OVERWRITE_COMP('s', sin)
      OVERWRITE_COMP('c', cos)
      OVERWRITE_COMP('t', tan)

    case '~': {
      _ drop = rsp->elem.matr.matrix;
      rsp->elem.matr = inverse_matrix(&rsp->elem.matr);
    } break;
    case '=':
      for (; rbp + 1 < rsp && elem_eq(rsp - 1, rsp); rsp--)
        ;
      (rbp + 1)->elem.real = rbp + 1 == rsp;
      if (rbp + 1 != rsp)
        rsp = rbp + 1;
      break;

    case 'a':
      switch (*++expr) {
        OVERWRITE_COMP('s', asin)
        OVERWRITE_COMP('c', acos)
        OVERWRITE_COMP('t', atan)
      default:
        disperr(__FUNCTION__, "unknown fn: %c", *expr);
      }
      break;

    case 'h':
      switch (*++expr) {
        OVERWRITE_COMP('s', sinh)
        OVERWRITE_COMP('c', cosh)
        OVERWRITE_COMP('t', tanh)
      default:
        disperr(__FUNCTION__, "unknown fn: %c", *expr);
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
      rsp->elem.comp *= M_PI / 180;
      break;
    case 'd': // to degree
      rsp->elem.comp *= 180 / M_PI;
      break;
    case 'm': // negative sign
      rsp->elem.comp *= -1;
      break;
    case 'i': // imaginary number
      rsp->elem.comp *= I;
      break;
    case 'p': { // polar
      double complex theta = (rsp--)->elem.comp;
      rsp->elem.comp =
          rsp->elem.comp * cos(theta) + I * rsp->elem.comp * sin(theta);
    } break;

    case '@': // system functions
      switch (*++expr) {
      case 'a': // ANS
        elem_set(++rsp, &info_c.hist[info_c.histi - 1]);
        break;
      case 'd':
        print_complex(rsp->elem.comp);
        break;
      case 'h': // history operation
        elem_set(rsp, &info_c.hist[info_c.histi - (size_t)rsp->elem.real - 1]);
        break;
      case 'n':
        elem_set(++rsp, &(elem_t){.rtype = RTYPE_COMP, .elem = {.comp = SNAN}});
        break;
      case 'p': // prev stack value
        rsp++;
        elem_set(rsp, rsp - 1);
        break;
      case 'r':
        (++rsp)->elem.comp = rand() / (double)RAND_MAX;
        break;
      case 's': // stack value operation
        elem_set(rsp, rsp - (int)rsp->elem.real - 1);
        break;
      default:
        [[clang::unlikely]];
      }
      break;

    case '\\': // special variables and CONSTANTS
      (++rsp)->elem.comp = get_const(*++expr);
      break;

    case '$': // register
      if (islower(*++expr)) [[clang::likely]] {
        elem_t *rhs = &info_c.reg[*expr - 'a'];
        elem_set(++rsp, rhs);
      }
      break;

    case '&':
      elem_set(&info_c.reg[*++expr - 'a'], rsp);
      break;

      // TODO differential
      // TODO integral
      // TODO sigma

    case ';': // comment
    case ',': // delimiter
      goto end;
    default:
      disperr(__FUNCTION__, "unknown char: %c", *expr);
    }
  }

end:
  if (rsp->rtype == RTYPE_MATR) {
    elem_t *rhs = &info_c.hist[info_c.histi++];
    if (rhs->rtype == RTYPE_MATR)
      nfree(rhs->elem.matr.matrix);
    *rhs = *rsp;
  } else if (info_c.histi < BUFSIZE)
    info_c.hist[info_c.histi++].elem.comp = rsp->elem.comp;
  set_rtinfo(info_c);
  return *rsp;
}

test(eval_expr_complex) {
  [[maybe_unused]] double complex result;

  result = eval_expr_complex("1 2i +").elem.comp;
  expecteq(1.0 + 2.0 * I, result);

  result = eval_expr_complex("4i 5 ^").elem.comp;
  expecteq(1024.0 * I, result);

  result = eval_expr_complex("1 1i + (2 2i +) *").elem.comp;
  expecteq(0 + 4.0 * I, result);

  // Test complex trigonometric functions
  result = eval_expr_complex("1 1i + s").elem.comp;
  expecteq(1.2984575814159773 + 0.6349639147847361 * I, result);

  // Test complex logarithm
  result = eval_expr_complex("\\P i l").elem.comp;
  expecteq(1.1447298858494002 + 1.5707963267948967 * I, result);

  matrix_t resultm;

  // Test matrix addition
  resultm = eval_expr_complex("[2 1,2,3,4,][2 5,6,7,8,]+").elem.matr;
  expecteq(6.0, resultm.matrix[0]);
  expecteq(8.0, resultm.matrix[1]);
  expecteq(10.0, resultm.matrix[2]);
  expecteq(12.0, resultm.matrix[3]);
  free(resultm.matrix);

  // Test matrix multiplication
  char const *expr = "[2 1,2,3,4,][2 5,6,7,8,]*";
  resultm = eval_expr_complex(expr).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(19.0, resultm.matrix[0]);
  expecteq(22.0, resultm.matrix[1]);
  expecteq(43.0, resultm.matrix[2]);
  expecteq(50.0, resultm.matrix[3]);
  free(resultm.matrix);

  // Test matrix inverse
  expr = "[2 1,2,3,4,]~";
  resultm = eval_expr_complex(expr).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(-2.0, resultm.matrix[0]);
  expecteq(1.0, resultm.matrix[1]);
  expecteq(1.5, resultm.matrix[2]);
  expecteq(-0.5, resultm.matrix[3]);
  free(resultm.matrix);

  // Scalar multiplication
  expr = "[3 5,6,7,] 5 *";
  resultm = eval_expr_complex(expr).elem.matr;
  expecteq(1, resultm.rows);
  expecteq(3, resultm.cols);
  expecteq(25, resultm.matrix[0]);
  expecteq(30, resultm.matrix[1]);
  expecteq(35, resultm.matrix[2]);
  free(resultm.matrix);
}

bench(eval_expr_complex) {
  eval_expr_complex("1 2 3 4 5 +");
  eval_expr_complex("4 5 ^");
  eval_expr_complex("1s2^(1c2^)+");
  eval_expr_complex("  5    6    10    - 5  /");
  eval_expr_complex("5");
  eval_expr_complex("@a");
  eval_expr_complex("10 &x");
  eval_expr_complex("$x 2 *");
  eval_expr_complex("2 3 ^ (4 5 *) + (6 7 /) -");
  eval_expr_complex("\\P 2 / s");
  eval_expr_complex("\\P 4 / c");
  eval_expr_complex("2 l2");
  eval_expr_complex("100 lc");
  eval_expr_complex("1 0 /");
}

/**
 * @brief Output elem_t in appropriate format
 * @param[in] elem Output comtent
 */
void print_elem(elem_t elem) {
  switch (elem.rtype) {
  case RTYPE_REAL:
    print_real(elem.elem.real);
    break;
  case RTYPE_COMP:
    print_complex(elem.elem.comp);
    break;
  case RTYPE_MATR:
    print_matrix(elem.elem.matr);
    free(elem.elem.matr.matrix);
    break;
  case RTYPE_LAMB:
    print_lambda(elem.elem.lamb);
    free(elem.elem.lamb);
    break;
  default:
    [[clang::unlikely]];
  }
}

/**
 * @brief Output value of type double
 * @param[in] result Output value
 */
void print_real(double result) {
  if (isint(result))
    printf("result: %lld\n", (long long)result);
  else
    printf("result: %lf\n", result);
}

/**
 * @brief Output value of type double complex
 * @param[in] result Output value
 */
void print_complex(double complex result) {
  printf("result: %lf + %lfi\n", creal(result), cimag(result));
}

/**
 * @brief Output value of type double complex in phasor view
 * @param[in] result Output value
 */
void print_complex_polar(double complex result) {
  double complex res = result;
  if (isnan(creal(res)) || isnan(cimag(res)))
    return;

  printf("result: %lf \\phasor %lf\n", cabs(res),
         atan2(cimag(res), creal(res)));
}

/**
 * @brief Output value of type matrix_t
 * @param[in] result Output value
 */
void print_matrix(matrix_t result) {
  for (size_t i = 0; i < result.rows; i++) {
    for (size_t j = 0; j < result.cols; j++)
      if (cimag(result.matrix[result.cols * i + j]) == 0)
        printf("\t%lf", creal(result.matrix[result.cols * i + j]));
      else {
        printf("\t%lf + %lfi", creal(result.matrix[result.cols * i + j]),
               cimag(result.matrix[result.cols * i + j]));
      }

    putchar('\n');
  }
}

[[gnu::nonnull]] void print_lambda(char const *restrict result) {
  printf("result: ");
  puts(result);
}

/**
 * @brief Process command
 * @param[in] cmd Command input
 */
[[gnu::nonnull]] void proc_cmds(char const *restrict cmd) {
  plotcfg_t pcfg = get_plotcfg();

  // TODO save registers
  switch (*cmd++) {
  case 't': // toggle
    switch (*cmd) {
    case 'c': // complex mode
      eval_f = eval_f == eval_expr_real ? eval_expr_complex : eval_expr_real;
      break;
    case 'p': // plotcfg explicit implicit
      pcfg.plotexpr = pcfg.plotexpr == plotexpr ? plotexpr_implicit : plotexpr;
      set_plotcfg(pcfg);
      break;
    default:
      [[clang::unlikely]];
    }
    break;
  case 'o': {
    char buf[BUFSIZE];
    strncpy(buf, cmd, BUFSIZE);
    optexpr(buf);
    puts(buf);
  } break;
  case 'p':
    skipspcs((char const **)&cmd);
    if (*cmd == '\0') {
      plotexpr(pcfg.prevexpr);
      break;
    }

    pcfg.plotexpr(cmd);
    strncpy(pcfg.prevexpr, cmd, BUFSIZE);
    set_plotcfg(pcfg);
    break;
  case 's': // settings
    switch (*cmd) {
    case 'p': // plot
      change_plotconfig(cmd + 1);
      break;
    default:
      [[clang::unlikely]];
    }
    break;
  default:
    disperr(__FUNCTION__, "unknown command: %c", *(cmd - 1));
  }
}
