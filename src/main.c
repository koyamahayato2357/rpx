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
#include "exception.h"
#include "graphplot.h"
#include "matop.h"
#include "optexpr.h"
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
  } else {
    elem_t res;
    try res = eval_f(input_buf);
    catchany capture(errcode) disperr(__FUNCTION__, codetomsg(errcode));
    print_elem(res);
  }
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
        disperr(__FUNCTION__, "Unknown option: %c", argv[i][1]);
        exit(1);
      }

    // interpreted as a file name
    FILE *fp = fopen(argv[i], "r");
    if (fp == nullptr) {
      disperr(__FUNCTION__, "File not found: %s", argv[i]);
      exit(1);
    }
    reader_loop(fp);
    fclose(fp);
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
 * @brief Evaluate real number expression
 * @param expr String of expression
 * @return Expression evaluation result
 * @throws ERR_UNKNOWN_FN ERR_UNKNOWN_CHAR
 */
elem_t eval_expr_real(char *expr) {
  double operand_stack[BUFSIZE] = {0};
  double *rsp = operand_stack, *rbp = operand_stack;
  rtinfo_t info = get_rtinfo('r');

  for (;; expr++) {
    if (isdigit(*expr))
      *++rsp = strtod(expr, &expr);
    if (isspace(*expr))
      continue;
    if (*expr == '\0')
      break;

    switch (*expr) {
    case '(': // start of sub-expr
      *++rsp = rbp - operand_stack;
      rbp = rsp;
      break;
    case ')': // end of sub-expr
      rbp = operand_stack + (int)*rbp;
      rsp--;
      *rsp = *(rsp + 1);
      break;

    case '+':
      for (; rbp + 1 < rsp; *(rbp + 1) += *rsp--)
        ;
      break;
    case '-':
      for (; rbp + 1 < rsp; *(rbp + 1) -= *rsp--)
        ;
      break;
    case '*':
      for (; rbp + 1 < rsp; *(rbp + 1) *= *rsp--)
        ;
      break;
    case '/':
      for (; rbp + 1 < rsp; *(rbp + 1) /= *rsp--)
        ;
      break;
    case '%':
      for (; rbp + 1 < rsp; *(rbp + 1) = fmod(*(rbp + 1), *rsp--))
        ;
      break;
    case '^':
      for (; rbp + 1 < rsp; *(rbp + 1) = pow(*(rbp + 1), *rsp--))
        ;
      break;

    case 'a':
      switch (*++expr) {
      case 's': // asin
        *rsp = asin(*rsp);
        break;
      case 'c': // acos
        *rsp = acos(*rsp);
        break;
      case 't': // atan
        *rsp = atan(*rsp);
        break;
      case 'v': { // average
        int n = rsp - rbp;
        for (; rsp > rbp + 1; *(rbp + 1) += *rsp--)
          ;
        *rsp /= n;
      } break;
      default:
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'h':
      switch (*++expr) {
      case 's': // sinh
        *rsp = sinh(*rsp);
        break;
      case 'c': // cosh
        *rsp = cosh(*rsp);
        break;
      case 't': // tanh
        *rsp = tanh(*rsp);
        break;
      default:
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'l':
      switch (*++expr) {
      case '2': // log2
        *rsp = log2(*rsp);
        break;
      case 'c': // common log
        *rsp = log10(*rsp);
        break;
      case 'e': // natural log
        *rsp = log(*rsp);
        break;
      default:
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'i':
      switch (*++expr) {
      case 'g': {
        double x = *rsp--;
        *rsp = gcd(*rsp, x);
      } break;
      case 'l': {
        double x = *rsp--;
        *rsp = lcm(*rsp, x);
      } break;
      case 'p': {
        double x = *rsp--;
        *rsp = permutation(*rsp, x);
      } break;
      case 'c': {
        double x = *rsp--;
        *rsp = combination(*rsp, x);
      } break;
      }
      break;

    case 'L': { // log with base
      double x = *rsp--;
      *rsp = log(*rsp) / log(x);
    } break;
    case 'A':
      *rsp = fabs(*rsp);
      break;
    case 'g':
      *rsp = tgamma(*rsp);
      break;
    case 'm': // negative sign
      *rsp *= -1;
      break;
    case 's': // sin
      *rsp = sin(*rsp);
      break;
    case 'c': // cos
      *rsp = cos(*rsp);
      break;
    case 't': // tan
      *rsp = tan(*rsp);
      break;
    case 'C':
      *rsp = ceil(*rsp);
      break;
    case 'F':
      *rsp = floor(*rsp);
      break;
    case 'R':
      *rsp = round(*rsp);
      break;
    case 'r': // to radian
      *rsp *= M_PI / 180;
      break;
    case 'd': // to degree
      *rsp *= 180 / M_PI;
      break;

    case '@': // system functions
      switch (*++expr) {
      case 'a': // ANS
        *++rsp = info.hist[info.histi - 1].elem.real;
        break;
      case 'h': // history operation
        *rsp = info.hist[info.histi - (int)*rsp - 1].elem.real;
        break;
      case 'p': // prev stack value
        rsp++;
        *rsp = *(rsp - 1);
        break;
      case 's': // stack value operation
        *rsp = *(rsp - (int)*rsp - 1);
        break;
      }
      break;

    case '\\': // special variables and CONSTANTS
      switch (*++expr) {
      case 'E': // Euler's
        *++rsp = M_E;
        break;
      case 'P': // pie
        *++rsp = M_PI;
        break;
      }
      break;

    case '$': // variable oparation
      if (islower(*++expr)) {
        char vname = *expr++;
        skipspcs(&expr);
        if (*expr == 'u') // update variable
          info.usrvar[vname - 'a'].elem.real = *rsp;
        else {
          *++rsp = info.usrvar[vname - 'a'].elem.real;
          expr--;
        }
      } else if (isdigit(*expr)) {
        *++rsp = info.usrfn.argv[*expr - '0' - 1];
      } else {
        switch (*expr) {
        case 'R': // random number between 0 and 1
          *++rsp = rand() / (double)RAND_MAX;
          break;
        }
      }
      break;

    case '!': // function operation
      if (islower(*++expr)) {
        int fname = *expr - 'a';
        rsp -= info.usrfn.argc[fname] - 1;
        memcpy(info.usrfn.argv, rsp, info.usrfn.argc[fname]);
        *rsp = eval_expr_real(info.usrfn.expr[fname]).elem.real;
      }
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
  if (info.histi < BUFSIZE)
    info.hist[info.histi++].elem.real = *rsp;
  set_rtinfo('r', info);
  return (elem_t){(result_t){.real = *rsp}, RTYPE_REAL};
}

test(eval_expr_real) {
  expect(double_eq(eval_expr_real("1 2 3 4 5 +").elem.real, 15.0));
  expect(double_eq(eval_expr_real("4 5 ^").elem.real, 1024.0));
  expect(double_eq(eval_expr_real("1s2^(1c2^)+").elem.real, 1.0));
  expect(double_eq(eval_expr_real("  5    6    10    - 5  /").elem.real, -2.2));

  // Test ANS functionality
  eval_expr_real("5");
  expect(double_eq(eval_expr_real("@a").elem.real, 5.0));

  // Test variable operations
  eval_expr_real("10 $x u");
  expect(double_eq(eval_expr_real("$x 2 *").elem.real, 20.0));

  // Test more complex expressions
  expect(double_eq(eval_expr_real("2 3 ^ (4 5 *) + (6 7 /) -").elem.real,
                   27.142857));

  // Test trigonometric functions
  expect(double_eq(eval_expr_real("\\P 2 / s").elem.real, 1.0));
  expect(double_eq(eval_expr_real("\\P 4 / c").elem.real, 0.707107));

  // Test logarithmic functions
  expect(double_eq(eval_expr_real("2 l2").elem.real, 1.0));
  expect(double_eq(eval_expr_real("100 lc").elem.real, 2.0));

  // Test error handling
  expect(isinf(eval_expr_real("1 0 /").elem.real));
}

/**
 * @brief Evaluate complex number expression
 * @param expr String of expression
 * @return elem_t Expression evaluation result
 */
elem_t eval_expr_complex(char *expr) {
  elem_t operand_stack[BUFSIZE] = {0};
  elem_t *rsp = operand_stack, *rbp = operand_stack;
  rtinfo_t info_c = get_rtinfo('c');

  for (;; expr++) {
    if (isdigit(*expr)) {
      (++rsp)->rtype = RTYPE_COMP;
      rsp->elem.comp = strtod(expr, &expr);
    }
    if (*expr == '[') {
      (++rsp)->rtype = RTYPE_MATR;
      expr++;
      matrix_t val = {.matrix = malloc(MAT_INITSIZE * sizeof(double complex))};
      matrix curelem = val.matrix;
      val.cols = strtol(expr, &expr, 10);
      for (; *expr != ']';) {
        *curelem++ = eval_expr_complex(expr).elem.comp;
        skip_untilcomma(&expr);
        skipspcs(&expr);
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

    case '+':
      while (rbp + 1 < rsp)
        elem_add((rbp + 1), rsp--);
      break;
    case '-':
      while (rbp + 1 < rsp)
        elem_sub((rbp + 1), rsp--);
      break;
    case '*':
      while (rbp + 1 < rsp)
        elem_mul((rbp + 1), rsp--);
      break;
    case '/':
      while (rbp + 1 < rsp)
        elem_div((rbp + 1), rsp--);
      break;
    case '^':
      while (rbp + 1 < rsp)
        elem_pow((rbp + 1), rsp--);
      break;
    case '~': {
      matrix temp = rsp->elem.matr.matrix;
      ignerr rsp->elem.matr = inverse_matrix(&rsp->elem.matr);
      free(temp);
    } break;

    case 'a':
      switch (*++expr) {
      case 's': // asin
        rsp->elem.comp = asin(rsp->elem.comp);
        break;
      case 'c': // acos
        rsp->elem.comp = acos(rsp->elem.comp);
        break;
      case 't': // atan
        rsp->elem.comp = atan(rsp->elem.comp);
        break;
      default:
        throw(ERR_UNKNOWN_FN);
      }
      break;

    case 'h':
      switch (*++expr) {
      case 's': // sinh
        rsp->elem.comp = sinh(rsp->elem.comp);
        break;
      case 'c': // cosh
        rsp->elem.comp = cosh(rsp->elem.comp);
        break;
      case 't': // tanh
        rsp->elem.comp = tanh(rsp->elem.comp);
        break;
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
    case 'A':
      rsp->elem.comp = fabs(rsp->elem.comp);
      break;
    case 'm': // negative sign
      rsp->elem.comp *= -1;
      break;
    case 'i': // imaginary number
      rsp->elem.comp *= 1i;
      break;
    case 's': // sin
      rsp->elem.comp = sin(rsp->elem.comp);
      break;
    case 'c': // cos
      rsp->elem.comp = cos(rsp->elem.comp);
      break;
    case 't': // tan
      rsp->elem.comp = tan(rsp->elem.comp);
      break;
    case 'r': // to radian
      rsp->elem.comp *= M_PI / 180;
      break;
    case 'd': // to degree
      rsp->elem.comp *= 180 / M_PI;
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
      }
      break;

    case '\\': // special variables and CONSTANTS
      switch (*++expr) {
      case 'E': // Euler's
        (++rsp)->elem.comp = M_E;
        break;
      case 'P': // pie
        (++rsp)->elem.comp = M_PI;
        break;
      }
      break;

    case '$': { // variable oparation
      if (islower(*++expr)) {
        char vname = *expr++;
        skipspcs(&expr);
        if (*expr == 'u') // update variable
          elem_set(&info_c.usrvar[vname - 'a'], rsp);
        else {
          elem_t *rhs = &info_c.usrvar[vname - 'a'];
          elem_set(++rsp, rhs);
          expr--;
        }
      } else {
        switch (*expr) {
        case 'R':
          (++rsp)->elem.comp = rand() / (double)RAND_MAX;
          break;
        }
      }
      break;
    }

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

  return *rsp;
}

test(eval_expr_complex) {
  double complex result;

  result = eval_expr_complex("1 2i +").elem.comp;
  expect(complex_eq(result, 1.0 + 2.0 * I));

  result = eval_expr_complex("4i 5 ^").elem.comp;
  expect(complex_eq(result, 1024.0 * I));

  result = eval_expr_complex("1 1i + (2 2i +) *").elem.comp;
  expect(complex_eq(result, 0 + 4.0 * I));

  // Test complex trigonometric functions
  result = eval_expr_complex("1 1i + s").elem.comp;
  expect(complex_eq(result, 1.2984575814159773 + 0.6349639147847361 * I));

  // Test complex logarithm
  result = eval_expr_complex("\\P i l").elem.comp;
  expect(complex_eq(result, 1.1447298858494002 + 1.5707963267948967 * I));

  matrix_t resultm;

  // Test matrix addition
  resultm = eval_expr_complex("[2 1,2,3,4,][2 5,6,7,8,]+").elem.matr;
  expect(resultm.rows == 2 && resultm.cols == 2);
  expect(double_eq(resultm.matrix[0], 6.0));
  expect(double_eq(resultm.matrix[1], 8.0));
  expect(double_eq(resultm.matrix[2], 10.0));
  expect(double_eq(resultm.matrix[3], 12.0));
  free(resultm.matrix);

  // Test matrix multiplication
  char expr2[] = "[2 1,2,3,4,][2 5,6,7,8,]*";
  resultm = eval_expr_complex(expr2).elem.matr;
  expect(resultm.rows == 2 && resultm.cols == 2);
  expect(double_eq(resultm.matrix[0], 19.0));
  expect(double_eq(resultm.matrix[1], 22.0));
  expect(double_eq(resultm.matrix[2], 43.0));
  expect(double_eq(resultm.matrix[3], 50.0));
  free(resultm.matrix);

  // Test matrix inverse
  char expr3[] = "[2 1,2,3,4,]~";
  resultm = eval_expr_complex(expr3).elem.matr;
  expect(resultm.rows == 2 && resultm.cols == 2);
  expect(double_eq(resultm.matrix[0], -2.0));
  expect(double_eq(resultm.matrix[1], 1.0));
  expect(double_eq(resultm.matrix[2], 1.5));
  expect(double_eq(resultm.matrix[3], -0.5));
  free(resultm.matrix);

  // Scalar multiplication
  char expr4[] = "[3 5,6,7,] 5 *";
  resultm = eval_expr_complex(expr4).elem.matr;
  expect(resultm.rows == 1 && resultm.cols == 3);
  expect(double_eq(resultm.matrix[0], 25));
  expect(double_eq(resultm.matrix[1], 30));
  expect(double_eq(resultm.matrix[2], 35));
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
    optexpr(BUFSIZE, cmd);
    strncpy(info_r.usrfn.expr[fname], cmd, BUFSIZE);
    set_plotcfg(pcfg);
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
    optexpr(BUFSIZE, buf);
    puts(buf);
  } break;
  case 'p':
    cmd++;
    skipspcs(&cmd);
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
