#pragma once

#include "gene.h"
#include "mathdef.h"
#include <stdlib.h>

#define dropmatr [[gnu::cleanup(free_matr)]]

constexpr size_t MAT_INITSIZE = 32;

typedef complex *matrix;

typedef struct {
  size_t rows;
  size_t cols;
  matrix matrix;
} matrix_t;

matrix_t new_matrix(size_t, size_t);
void free_matr(matrix_t *restrict);
bool meq(matrix_t const *, matrix_t const *);
matrix_t madd(matrix_t const *, matrix_t const *);
matrix_t msub(matrix_t const *, matrix_t const *);
matrix_t mmul(matrix_t const *, matrix_t const *);
double det(matrix_t const *);
matrix_t inverse_matrix(matrix_t const *);
void smul(matrix_t *, complex);
overloadable bool eq(matrix_t const *, matrix_t const *);
