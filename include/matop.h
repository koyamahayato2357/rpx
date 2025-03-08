/**
 * @file include/matop.h
 */

#pragma once

#include "gene.h"
#include "mathdef.h"
#include <stdlib.h>

#define dropmatr [[gnu::cleanup(freeMatr)]]

constexpr size_t mat_init_size = 32;

typedef complex *matrix;

typedef struct {
  size_t rows;
  size_t cols;
  matrix matrix;
} matrix_t;

[[nodiscard("allocation")]] matrix_t newMatrix(size_t, size_t);
[[gnu::nonnull]] void freeMatr(matrix_t *restrict);
[[gnu::nonnull]] bool mEq(matrix_t const *, matrix_t const *);

[[nodiscard("allocation"), gnu::nonnull]] matrix_t
mAdd(matrix_t const *, matrix_t const *);

[[nodiscard("allocation"), gnu::nonnull]] matrix_t
mSub(matrix_t const *, matrix_t const *);

[[nodiscard("allocation"), gnu::nonnull]] matrix_t
mMul(matrix_t const *, matrix_t const *);

[[nodiscard("allocation"), gnu::nonnull]] matrix_t
inverseMatrix(matrix_t const *);

[[gnu::nonnull]] void smul(matrix_t *, complex);
overloadable bool eq(matrix_t const *, matrix_t const *);
