#include "matop.h"
#include "arthfn.h"
#include "chore.h"
#include "errcode.h"
#include "exception.h"
#include "gene.h"

matrix_t NAN_matrix(size_t rows, size_t cols) {
  matrix_t result = new_matrix(rows, cols);
  for (size_t i = 0; i < rows * cols; i++)
    result.matrix[i] = SNAN;
  return result;
}

matrix_t new_matrix(size_t rows, size_t cols) {
  return (matrix_t){.rows = rows,
                    .cols = cols,
                    .matrix = palloc(rows * cols * sizeof(double complex))};
}

bool mcheckdim(matrix_t *lhs, matrix_t *rhs) {
  return lhs->rows == rhs->rows && lhs->cols == rhs->cols;
}

bool meq(matrix_t *lhs, matrix_t *rhs) {
  if (!mcheckdim(lhs, rhs))
    return false;

  for (size_t i = 0; i < lhs->cols * lhs->rows; i++)
    if (!eq(lhs->matrix[i], rhs->matrix[i]))
      return false;

  return true;
}

/**
 * @brief Add/Sub between matrices
 * @throws ERR_DIMENTION_MISMATCH
 */
#define MOPS(name, op)                                                      \
  matrix_t m##name(matrix_t *lhs, matrix_t *rhs) {                             \
    if (!mcheckdim(lhs, rhs))                                                  \
      throw(ERR_DIMENTION_MISMATCH);                                           \
                                                                               \
    matrix_t result = new_matrix(lhs->rows, lhs->cols);                        \
                                                                               \
    for (size_t i = 0; i < lhs->rows * lhs->cols; i++)                         \
      result.matrix[i] = lhs->matrix[i] op rhs->matrix[i];                     \
                                                                               \
    return result;                                                             \
  }
MOPS(add, +)
MOPS(sub, -)

/**
 * @brief Mul between matrices
 * @throws ERR_DIMENTION_MISMATCH
 */
matrix_t mmul(matrix_t *lhs, matrix_t *rhs) {
  if (lhs->rows != rhs->cols && lhs->cols != rhs->rows)
    throw(ERR_DIMENTION_MISMATCH);

  matrix_t result = new_matrix(lhs->rows, rhs->cols);

  for (size_t i = 0; i < lhs->rows; i++)
    for (size_t j = 0; j < rhs->cols; j++)
      for (size_t k = 0; k < lhs->cols; k++)
        result.matrix[rhs->cols * i + j] +=
            lhs->matrix[lhs->cols * i + k] * rhs->matrix[rhs->cols * k + j];

  return result;
}

/**
 * @brief Calculate determinant
 * @throws ERR_NON_SQUARE_MATRIX
 */
double det(matrix_t *A) {
  if (A->rows != A->cols)
    throw(ERR_NON_SQUARE_MATRIX);

  double result = 1;
  int dim = A->rows;
  for (int i = 0; i < dim - 1; i++)
    for (int j = 0; j < dim - 1; j++) {
      for (int k = 1; !A->matrix[dim * i + i]; k++) {
        for (int l = i; l < dim; l++) {
          if (k >= dim) // case of singular matrix
            return 0;

          double temp = A->matrix[dim * i + l];
          A->matrix[dim * i + l] = A->matrix[dim * (i + k) + l];
          A->matrix[dim * (i + k) + l] = -temp;
        }
      }
      // elimination
      double temp = A->matrix[dim * (j + 1) + i] / A->matrix[dim * i + i];
      for (int k = i; k < dim; k++)
        A->matrix[dim * (j + 1) + k] -= temp * A->matrix[dim * i + k];
    }

  for (int i = 0; i < dim; i++)
    result *= A->matrix[dim * i + i];
  return result;
}

/**
 * @brief Calculate inverse matrix with Gaussian elimination
 * @param[in] A Matrix
 * @return Inverted A
 */
matrix_t inverse_matrix(matrix_t *A) {
  int dim = A->rows;

  if (A->rows != A->cols)
    throw(ERR_NON_SQUARE_MATRIX);

  matrix_t result = new_matrix(dim, dim);

  for (int i = 0; i < dim; i++)
    result.matrix[i * dim + i] = 1;

  // to diagonal matrix
  for (int i = 0; i < dim; i++) {
    if (A->matrix[i * dim + i] == 0) {
      int j;
      for (j = (i + 1) % dim; A->matrix[j * dim + i] == 0; j = (j + 1) % dim)
        if (j == i) {
          free(result.matrix);
          throw(ERR_IRREGULAR_MATRIX);
        }

      for (int k = 0; k < dim; k++) {
        double complex temp = A->matrix[i * dim + k];
        A->matrix[i * dim + k] = A->matrix[j * dim + k];
        A->matrix[j * dim + k] = -temp;
        temp = result.matrix[i * dim + k];
        result.matrix[i * dim + k] = result.matrix[j * dim + k];
        result.matrix[j * dim + k] = -temp;
      }
    }

    for (int j = (i + 1) % dim; j != i; j = (j + 1) % dim) {
      double complex coef = A->matrix[j * dim + i] / A->matrix[i * dim + i];
      for (int k = 0; k < dim; k++) {
        unsigned int id = (k + i) % dim;
        A->matrix[j * dim + id] -= coef * A->matrix[i * dim + id];
        result.matrix[j * dim + id] -= coef * result.matrix[i * dim + id];
      }
    }
  }

  // A->matrix to unit matrix
  for (int i = 0; i < dim; i++)
    for (int j = 0; j < dim; j++) {
      if (A->matrix[i * dim + i] == 0) {
        free(result.matrix);
        throw(ERR_IRREGULAR_MATRIX);
      }

      result.matrix[i * dim + j] /= A->matrix[i * dim + i];
    }

  return result;
}

/**
 * @brief Scalar mul
 * @param[in/out] lhs Matrix
 * @param[in] rhs Scalar
 */
void smul(matrix_t *lhs, double complex rhs) {
  for (size_t i = 0; i < lhs->rows * lhs->cols; i++)
    lhs->matrix[i] *= rhs;
}
