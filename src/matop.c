/**
 * @file src/matop.c
 * @brief Define matrix related functions
 */

#include "matop.h"
#include "benchmarking.h"
#include "chore.h"
#include "errcode.h"
#include "error.h"
#include "gene.h"

static matrix_t NAN_matrix(size_t rows, size_t cols) {
  matrix_t result = new_matrix(rows, cols);
  for (size_t i = 0; i < rows * cols; i++) result.matrix[i] = NAN;
  return result;
}

matrix_t new_matrix(size_t rows, size_t cols) {
  // formatter broken...
  return (matrix_t
  ){.rows = rows, .cols = cols, .matrix = zalloc(complex, rows * cols)};
}

void free_matr(matrix_t *restrict x) {
  free(x->matrix);
}

static bool mcheckdim(matrix_t const *lhs, matrix_t const *rhs) {
  return lhs->rows == rhs->rows && lhs->cols == rhs->cols;
}

bool meq(matrix_t const *lhs, matrix_t const *rhs) {
  if (!mcheckdim(lhs, rhs)) return false;

  for (size_t i = 0; i < lhs->cols * lhs->rows; i++)
    if (!eq(lhs->matrix[i], rhs->matrix[i])) return false;

  return true;
}

overloadable bool eq(matrix_t const *lhs, matrix_t const *rhs) {
  return meq(lhs, rhs);
}

/**
 * @brief Add/Sub between matrices
 */
#define MOPS(name, op) \
  matrix_t m##name( \
    matrix_t const *restrict lhs, matrix_t const *restrict rhs \
  ) { \
    if (!mcheckdim(lhs, rhs)) { \
      disperr( \
        __FUNCTION__, \
        "%s: %zux%zu vs %zux%zu", \
        codetomsg(ERR_DIMENTION_MISMATCH), \
        lhs->rows, \
        lhs->cols, \
        rhs->rows, \
        rhs->cols \
      ); \
      return NAN_matrix(lhs->rows, lhs->cols); \
    } \
    matrix_t result = new_matrix(lhs->rows, lhs->cols); \
    for (size_t i = 0; i < lhs->rows * lhs->cols; i++) \
      result.matrix[i] = lhs->matrix[i] op rhs->matrix[i]; \
    return result; \
  }
APPLY_ADDSUB(MOPS)

/**
 * @brief Mul between matrices
 */
matrix_t mmul(matrix_t const *restrict lhs, matrix_t const *restrict rhs) {
  if (lhs->rows != rhs->cols && lhs->cols != rhs->rows) [[clang::unlikely]] {
    disperr(
      __FUNCTION__,
      "%s: %zux%zu vs %zux%zu",
      codetomsg(ERR_DIMENTION_MISMATCH),
      lhs->rows,
      lhs->cols,
      rhs->rows,
      rhs->cols
    );
    return NAN_matrix(lhs->rows, lhs->cols);
  }

  matrix_t result = new_matrix(lhs->rows, rhs->cols);

  for (size_t i = 0; i < lhs->rows; i++)
    for (size_t j = 0; j < rhs->cols; j++)
      for (size_t k = 0; k < lhs->cols; k++)
        result.matrix[rhs->cols * i + j]
          += lhs->matrix[lhs->cols * i + k] * rhs->matrix[rhs->cols * k + j];

  return result;
}

/**
 * @brief Calculate determinant
 * @note No benefit of vectorization in the current implementation
 */
static double det(matrix_t const *restrict A) {
  if (A->rows != A->cols) [[clang::unlikely]] {
    disperr(__FUNCTION__, "%s", codetomsg(ERR_NON_SQUARE_MATRIX));
    return 0;
  }

  double result = 1;
  size_t dim = A->rows;
  for (size_t i = 0; i < dim - 1; i++)
    for (size_t j = 0; j < dim - 1; j++) {
      for (size_t k = 1; eq(0.0i, A->matrix[dim * i + i]); k++) {
        for (size_t l = i; l < dim; l++) {
          if (k >= dim) [[clang::unlikely]] // case of singular matrix
            return 0;

          double temp = creal(A->matrix[dim * i + l]);
          A->matrix[dim * i + l] = A->matrix[dim * (i + k) + l];
          A->matrix[dim * (i + k) + l] = -temp;
        }
      }
      // elimination
      double temp
        = creal(A->matrix[dim * (j + 1) + i] / A->matrix[dim * i + i]);
      for (size_t k = i; k < dim; k++)
        A->matrix[dim * (j + 1) + k] -= temp * A->matrix[dim * i + k];
    }

  for (size_t i = 0; i < dim; i++) result *= A->matrix[dim * i + i];
  return result;
}

// for vectorization benchmarking
bench (det2x2) {
  complex m[] = {3, 5, 2, 7};
  matrix_t A = {.rows = 2, .cols = 2, .matrix = m};
  det(&A);
}
bench (det3x3) {
  complex m[] = {3, 5, 2, 7, 6, 1, 9, 6};
  matrix_t A = {.rows = 3, .cols = 3, .matrix = m};
  det(&A);
}

/**
 * @brief Calculate inverse matrix with Gaussian elimination
 * @param[in] A Matrix
 * @return Inverted A
 */
matrix_t inverse_matrix(matrix_t const *restrict A) {
  size_t dim = A->rows;

  if (A->rows != A->cols) [[clang::unlikely]] {
    disperr(__FUNCTION__, "%s", codetomsg(ERR_NON_SQUARE_MATRIX));
    return *A;
  }

  matrix_t result = new_matrix(dim, dim);

  for (size_t i = 0; i < dim; i++) result.matrix[i * dim + i] = 1;

  // to diagonal matrix
  for (size_t i = 0; i < dim; i++) {
    if (A->matrix[i * dim + i] == 0) {
      size_t j;
      for (j = (i + 1) % dim; A->matrix[j * dim + i] == 0; j = (j + 1) % dim)
        if (j == i) [[clang::unlikely]] {
          free(result.matrix);
          disperr(__FUNCTION__, "%s", codetomsg(ERR_IRREGULAR_MATRIX));
          return *A;
        }

      for (size_t k = 0; k < dim; k++) {
        complex temp = A->matrix[i * dim + k];
        A->matrix[i * dim + k] = A->matrix[j * dim + k];
        A->matrix[j * dim + k] = -temp;
        temp = result.matrix[i * dim + k];
        result.matrix[i * dim + k] = result.matrix[j * dim + k];
        result.matrix[j * dim + k] = -temp;
      }
    }

    for (size_t j = (i + 1) % dim; j != i; j = (j + 1) % dim) {
      complex coef = A->matrix[j * dim + i] / A->matrix[i * dim + i];
      for (size_t k = 0; k < dim; k++) {
        size_t id = (k + i) % dim;
        A->matrix[j * dim + id] -= coef * A->matrix[i * dim + id];
        result.matrix[j * dim + id] -= coef * result.matrix[i * dim + id];
      }
    }
  }

  // A->matrix to unit matrix
  for (size_t i = 0; i < dim; i++)
    for (size_t j = 0; j < dim; j++) {
      if (eq(fabs(A->matrix[i * dim + i]), 0.0)) [[clang::unlikely]] {
        free(result.matrix);
        disperr(__FUNCTION__, "%s", codetomsg(ERR_IRREGULAR_MATRIX));
        return *A;
      }

      result.matrix[i * dim + j] /= A->matrix[i * dim + i];
    }

  return result;
}

/**
 * @brief Scalar mul
 * @param[in,out] lhs Matrix
 * @param[in] rhs Scalar
 */
void smul(matrix_t *restrict lhs, complex rhs) {
  for (size_t i = 0; i < lhs->rows * lhs->cols; i++) lhs->matrix[i] *= rhs;
}
