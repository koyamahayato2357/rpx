#include "elem_operations.h"
#include "chore.h"
#include "errcode.h"
#include "exception.h"
#include "matrix_operations.h"
#include <assert.h>
#include <stdbit.h>
#include <string.h>
#include <tgmath.h>

void elem_set(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_MATR)
    free(lhs->elem.matr.matrix);

  *lhs = *rhs;
}

void elem_add(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype != rhs->rtype)
    throw(ERR_TYPE_MISMATCH);

  if (rhs->rtype == RTYPE_MATR) {
    matrix temp = lhs->elem.matr.matrix;

    lhs->elem.matr = madd(&lhs->elem.matr, &rhs->elem.matr);

    nfree(temp);
    nfree(rhs->elem.matr.matrix);
    return;
  }

  lhs->elem.comp += rhs->elem.comp;
}

void elem_sub(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype != rhs->rtype)
    throw(ERR_TYPE_MISMATCH);

  if (rhs->rtype == RTYPE_MATR) {
    matrix temp = lhs->elem.matr.matrix;

    lhs->elem.matr = msub(&lhs->elem.matr, &rhs->elem.matr);

    nfree(temp);
    nfree(rhs->elem.matr.matrix);
    return;
  }

  lhs->elem.comp -= rhs->elem.comp;
}

rtype_t elem_mul(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_MATR && rhs->rtype == RTYPE_MATR) {
    matrix temp = lhs->elem.matr.matrix;

    lhs->elem.matr = mmul(&lhs->elem.matr, &rhs->elem.matr);

    nfree(temp);
    nfree(rhs->elem.matr.matrix);
    return RTYPE_MATR;
  } else if (lhs->rtype == RTYPE_MATR && rhs->rtype == RTYPE_COMP) {
    smul(&lhs->elem.matr, rhs->elem.comp);
    return RTYPE_MATR;
  } else if (lhs->rtype == RTYPE_COMP && rhs->rtype == RTYPE_MATR) {
    smul(&rhs->elem.matr, lhs->elem.comp);
    *lhs = *rhs;
    return RTYPE_MATR;
  }

  lhs->elem.comp *= rhs->elem.comp;
  return RTYPE_COMP;
}

void elem_div(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_COMP)
    lhs->elem.comp /= rhs->elem.comp;
  else
    smul(&lhs->elem.matr, 1 / rhs->elem.comp);
}

void elem_pow(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_COMP)
    lhs->elem.comp = pow(lhs->elem.comp, rhs->elem.comp);
  else {
    if (creal(rhs->elem.comp) < 0) {
      matrix_t temp;

      temp = inverse_matrix(&lhs->elem.matr);

      free(lhs->elem.matr.matrix);
      lhs->elem.matr = temp;
      rhs->elem.comp *= -1;
    }

    unsigned long long n = (unsigned long long)creal(rhs->elem.comp);
    matrix_t A = new_matrix(lhs->elem.matr.rows, lhs->elem.matr.cols);
    memcpy(A.matrix, lhs->elem.matr.matrix,
           A.rows * A.cols * sizeof(double complex));
    for (size_t i = 1; i < n; i++) {
      matrix_t temp = mmul(&lhs->elem.matr, &A);
      free(lhs->elem.matr.matrix);
      lhs->elem.matr = temp;
    }
    free(A.matrix);
  }
}
