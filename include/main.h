/**
 * @file include/main.h
 * @brief Define macros and data types for evaluating expr of complex number
 */

#pragma once
#include "matop.h"
#include <stdio.h>

constexpr size_t buf_size = 64;
#define OP_CASE_ELEM(tok, op) \
  case *#op: \
    while (rbp + 1 < rsp) elem##tok(rbp + 1, rsp--); \
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

void procAList(int, char const **);
void readerLoop(FILE *);
elem_t evalExprComplex(char const *);
void printElem(elem_t);
void printReal(double);
void printComplexComplex(complex);
void printComplexPolar(complex);
void printMatrix(matrix_t);
void printLambda(char const *);
void procCmds(char const *);
overloadable void printany(elem_t);
