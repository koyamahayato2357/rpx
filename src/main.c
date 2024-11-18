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
#include "chore.h"
#include "editline.h"
#include "elemop.h"
#include "errcode.h"
#include "error.h"
#include "evalfn.h"
#include "exception.h"
#include "exproriented.h"
#include "graphplot.h"
#include "matop.h"
#include "optexpr.h"
#include "phyconst.h"
#include "rc.h"
#include "sysconf.h"
#include "testing.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>

auto eval_f = eval_expr_real;

int main(int argc, char **argv) {
  srand(time(nullptr));

  init_plotconfig();
  load_initscript(nullptr);

  proc_alist(argc, argv);

  reader_loop_stdin();

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
  puts("  - Variable storage ($a to $z)");
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
void proc_input(char *input_buf) {
  if (*input_buf == ':') {
    ignerr proc_cmds(input_buf + 1);
    return;
  }
  elem_t res;
  try res = eval_f(input_buf);
  catchany capture(e) disperr(__FUNCTION__, codetomsg(e));
  print_elem(res);
}

/**
 * @brief Process argument list
 * @param[in] argc arg count
 * @param[in] argv arg value
 */
void proc_alist(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') // interpreted as a option
      switch (argv[i][1]) {
      case 'h':
        startup_message();
        continue;
      case 'r':
        proc_input(argv[++i]);
        continue;
      case 'q':
        exit(0);
      default:
        panic(ERR_UNKNOWN_OPTION, "%c ", argv[i][1]);
      }

    // interpreted as a file name
    FILE *fp dropfile =
        fopen(argv[i], "r") ?: p$panic(ERR_FILE_NOT_FOUND, "%s ", argv[i]);
    reader_loop(fp);
  }
}

/**
 * @brief File reading loop
 * @param[in] fp File stream
 */
void reader_loop(FILE *fp) {
  char input_buf[BUFSIZE];
  while (fgets(input_buf, BUFSIZE, fp) != nullptr)
    proc_input(input_buf);
}

//! @brief Interactive input loop
void reader_loop_stdin() {
  char input_buf[BUFSIZE] = {};
  while (editline(BUFSIZE, input_buf))
    proc_input(input_buf);
}

/**
 * @brief Evaluate complex number expression
 * @param expr String of expression
 * @return elem_t Expression evaluation result
 */
elem_t eval_expr_complex(char const *expr) {
  elem_t operand_stack[BUFSIZE] = {0};
  elem_t *rsp = operand_stack, *rbp = operand_stack;
  rtinfo_t info_c = get_rtinfo('c');

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
      val.cols = strtol(expr, (char **)&expr, 10);
      for (; *expr != ']';) {
        *curelem++ = eval_expr_complex(expr).elem.comp;
        skip_untilcomma(&expr);
      }
      val.rows = (curelem - val.matrix) / val.cols;
      rsp->elem.matr = val;
      continue;
    }
    if (isspace(*expr))
      continue;
    if (*expr == '\0')
      break;

    switch (*expr) {
    case '(':
      (++rsp)->elem.real = rbp - operand_stack;
      rbp = rsp;
      break;
    case ')':
      rbp = operand_stack + (int)rbp->elem.real;
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
      matrix temp drop = rsp->elem.matr.matrix;
      ignerr rsp->elem.matr = inverse_matrix(&rsp->elem.matr);
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
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'h':
      switch (*++expr) {
        OVERWRITE_COMP('s', sinh)
        OVERWRITE_COMP('c', cosh)
        OVERWRITE_COMP('t', tanh)
      default:
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'l':
      rsp->elem.comp = clog(rsp->elem.comp);
      break;

    case 'L': { // log with base
      double x = (rsp--)->elem.comp;
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
      rsp->elem.comp *= 1i;
      break;
    case 'p': { // polar
      double complex theta = (rsp--)->elem.comp;
      rsp->elem.comp =
          rsp->elem.comp * cos(theta) + I * rsp->elem.comp * sin(theta);
    }

    case '@': // system functions
      switch (*++expr) {
      case 'a': // ANS
        elem_set(++rsp, &info_c.hist[info_c.histi - 1]);
        break;
      case 'h': // history operation
        elem_set(rsp, &info_c.hist[info_c.histi - (int)rsp->elem.real - 1]);
        break;
      case 'p': // prev stack value
        rsp++;
        elem_set(rsp, rsp - 1);
        break;
      case 's': // stack value operation
        elem_set(rsp, rsp - (int)rsp->elem.real - 1);
        break;
      case 'r':
        (++rsp)->elem.comp = rand() / (double)RAND_MAX;
        break;
      }
      break;

    case '\\': // special variables and CONSTANTS
      (++rsp)->elem.comp = get_const(*++expr);
      break;

    case '$': // variable oparation
      if (islower(*++expr)) {
        elem_t *rhs = &info_c.usrvar[*expr - 'a'];
        elem_set(++rsp, rhs);
      }
      break;

    case '&':
      elem_set(&info_c.usrvar[*++expr - 'a'], rsp);
      break;

      // TODO differential
      // TODO integral
      // TODO sigma

    case ';': // comment
    case ',': // delimiter
      goto end;
    default:
      throw(ERR_UNKNOWN_CHAR);
    }
  }

end:
  if (rsp->rtype == RTYPE_MATR) {
    elem_t *rhs = &info_c.hist[info_c.histi++];
    if (rhs->rtype == RTYPE_MATR)
      nfree(rhs->elem.matr.matrix);
    *rhs = *rsp;
  } else
    info_c.hist[info_c.histi++].elem.comp = rsp->elem.comp;
  set_rtinfo('c', info_c);
  return *rsp;
}

test(eval_expr_complex) {
  double complex result;

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
  char expr2[] = "[2 1,2,3,4,][2 5,6,7,8,]*";
  resultm = eval_expr_complex(expr2).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(19.0, resultm.matrix[0]);
  expecteq(22.0, resultm.matrix[1]);
  expecteq(43.0, resultm.matrix[2]);
  expecteq(50.0, resultm.matrix[3]);
  free(resultm.matrix);

  // Test matrix inverse
  char expr3[] = "[2 1,2,3,4,]~";
  resultm = eval_expr_complex(expr3).elem.matr;
  expecteq(2, resultm.rows);
  expecteq(2, resultm.cols);
  expecteq(-2.0, resultm.matrix[0]);
  expecteq(1.0, resultm.matrix[1]);
  expecteq(1.5, resultm.matrix[2]);
  expecteq(-0.5, resultm.matrix[3]);
  free(resultm.matrix);

  // Scalar multiplication
  char expr4[] = "[3 5,6,7,] 5 *";
  resultm = eval_expr_complex(expr4).elem.matr;
  expecteq(1, resultm.rows);
  expecteq(3, resultm.cols);
  expecteq(25, resultm.matrix[0]);
  expecteq(30, resultm.matrix[1]);
  expecteq(35, resultm.matrix[2]);
  free(resultm.matrix);
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
  }
}

/**
 * @brief Output value of type double
 * @param[in] result Output value
 */
void print_real(double result) {
  if (isnan(result))
    return;

  if (isint(result) && result < (double)LLONG_MAX && result > (double)LLONG_MIN)
    printf("result: %lld\n", (long long)result);
  else
    printf("result: %lf\n", result);
}

/**
 * @brief Output value of type double complex
 * @param[in] result Output value
 */
void print_complex(double complex result) {
  if (isnan(creal(result)) || isnan(cimag(result)))
    return;

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
  // NAN check
  for (size_t i = 0; i < result.rows * result.cols; i++)
    if (isnan(creal(result.matrix[i])) || isnan(cimag(result.matrix[i])))
      return;

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

/**
 * @brief Process command
 * @param[in] cmd Command input
 */
void proc_cmds(char *cmd) {
  rtinfo_t info_r = get_rtinfo('r');
  plotcfg_t pcfg = get_plotcfg();

  // TODO save variables
  switch (*cmd) {
  case 'd': // define user function
    cmd++;
    int fname = *cmd++ - 'a';
    info_r.usrfn.argc[fname] = *cmd++ - '0';
    optexpr(cmd);
    strncpy(info_r.usrfn.expr[fname], cmd, BUFSIZE);
    set_rtinfo('r', info_r);
    break;
  case 't': // toggle
    switch (*++cmd) {
    case 'c': // complex mode
      eval_f = eval_f == eval_expr_real ? eval_expr_complex : eval_expr_real;
      break;
    case 'p': // plotcfg explicit implicit
      pcfg.plotexpr = pcfg.plotexpr == plotexpr ? plotexpr_implicit : plotexpr;
      set_plotcfg(pcfg);
      break;
    }
    break;
  case 'o': {
    char buf[BUFSIZE];
    strncpy(buf, cmd + 1, BUFSIZE);
    optexpr(buf);
    puts(buf);
  } break;
  case 'p':
    cmd++;
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
    switch (*++cmd) {
    case 'p': // plot
      change_plotconfig(cmd + 1);
      break;
    }
    break;
  default:
    throw(ERR_UNKNOWN_COMMAND);
  }
}
