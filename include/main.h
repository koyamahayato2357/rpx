/**
 * @file include/main.h
 * @brief Define macros and data types for evaluating expr of complex number
 */

#pragma once
#include "matop.h"
#include <stdio.h>

constexpr size_t BUFSIZE = 64;
#define OP_CASE_ELEM(tok, op) \
  case *#op: \
    while (rbp + 1 < rsp) elem_##tok(rbp + 1, rsp--); \
    break;
#define OVERWRITE(cas, var, fn) \
  case cas: \
    var = fn(var); \
    break;
// overwrite the top of the stack with the function in the parameter
#define OVERWRITE_REAL(cas, fn) OVERWRITE(cas, ei->s.rsp->elem.real, fn)
#define OVERWRITE_COMP(cas, fn) OVERWRITE(cas, rsp->elem.comp, fn)

//! @brief Set of types to handle
typedef enum {
  RTYPE_REAL = 0x01,
  RTYPE_COMP = 0x02,
  RTYPE_MATR = 0x04,
  RTYPE_LAMB = 0x08,
} rtype_t /* result type */;

//! @brief Wrapper of types to handle
typedef union {
  double real;
  complex comp;
  matrix_t matr;
  char *lamb;
} result_t;

//! @brief Tagged union of types to handle
typedef struct {
  result_t elem;
  rtype_t rtype;
} elem_t;

typedef struct {
  union {
    double real;
    char *lamb;
  } elem;
  bool isnum;
} real_t;

void proc_alist(int, char const **);
void reader_loop(FILE *);
elem_t eval_expr_complex(char const *);
void print_elem(elem_t);
void print_real(double);
void print_complex_complex(complex);
void print_complex_polar(complex);
void print_matrix(matrix_t);
void print_lambda(char const *);
void proc_cmds(char const *);
overloadable void printany(elem_t);
