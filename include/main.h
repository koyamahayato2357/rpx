//! @file main.h

#pragma once
#include <stdio.h>
#define BUFSIZE 256

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
elem_t eval_expr_real(char *);
elem_t eval_expr_complex(char *);
void print_elem(elem_t);
void print_real(double);
void print_complex(double complex);
void print_matrix(matrix_t);
void proc_cmds(char *);
