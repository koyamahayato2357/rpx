//! @file main.h

#pragma once
#include <stdio.h>
#define BUFSIZE 256
#define OP_CASE_ARTHM(op)                                                      \
  case *#op:                                                                   \
    for (; rbp + 1 < rsp; rbp[1] op## = *rsp--)                                \
      ;                                                                        \
    break;
#define OP_CASE_ADV(op, f)                                                     \
  case *#op:                                                                   \
    for (; rbp + 1 < rsp; rbp[1] = f(rbp[1], *rsp--))                          \
      ;                                                                        \
    break;
#define OP_CASE_EQA(op)                                                        \
  case *#op:                                                                   \
    for (; rbp + 1 < rsp && *(rsp - 1) op## = *rsp; rsp--)                     \
      ;                                                                        \
    *(rbp + 1) = rbp + 1 == rsp;                                               \
    rsp = rbp + 1;                                                             \
    break;
#define OP_CASE_ELEM(tok, op)                                                  \
  case *#op:                                                                   \
    while (rbp + 1 < rsp)                                                      \
      elem_##tok(rbp + 1, rsp--);                                              \
    break;
#define OVERWRITE(cas, var, fn)                                                \
  case cas:                                                                    \
    var = fn(var);                                                             \
    break;
#define OVERWRITE_REAL(cas, fn) OVERWRITE(cas, *rsp, fn)
#define OVERWRITE_COMP(cas, fn) OVERWRITE(cas, rsp->elem.comp, fn)

#include "matop.h"

//! @brief Set of types to handle
typedef enum {
  RTYPE_REAL = 0x01,
  RTYPE_COMP = 0x02,
  RTYPE_MATR = 0x04,
} rtype_t /* result type */;

//! @brief Wrapper of types to handle
typedef union {
  double real;
  double complex comp;
  matrix_t matr;
} result_t;

//! @brief Tagged union of types to handle
typedef struct {
  result_t elem;
  rtype_t rtype;
} elem_t;

void proc_alist(int, char **);
void reader_loop(FILE *);
void reader_loop_stdin();
elem_t eval_expr_complex(char const *);
void print_elem(elem_t);
void print_real(double);
void print_complex(double complex);
void print_matrix(matrix_t);
void proc_cmds(char *);
