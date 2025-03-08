/**
 * @file src/elemop.c
 * @brief Define elem_t related functions
 */

#include "elemop.h"
#include "chore.h"
#include "errcode.h"
#include "error.h"
#include "gene.h"
#include <string.h>

[[gnu::nonnull]] void
elemSet(elem_t *restrict lhs, elem_t const *restrict rhs) {
  if (lhs->rtype == RTYPE_MATR) free(lhs->elem.matr.matrix);
  *lhs = *rhs;
}

[[gnu::nonnull]] bool elemEq(elem_t const *lhs, elem_t const *rhs) {
  if (lhs == rhs) return true;
  if (lhs->rtype != rhs->rtype) return false;
  if (lhs->rtype == RTYPE_COMP) return eq(lhs->elem.comp, rhs->elem.comp);
  if (lhs->rtype == RTYPE_MATR) return mEq(&lhs->elem.matr, &rhs->elem.matr);
  return false;
}

void elemAdd(elem_t *lhs, elem_t const *rhs) {
  if (lhs->rtype != rhs->rtype) {
    dispErr(__FUNCTION__, "%s", codetomsg(ERR_TYPE_MISMATCH));
    return;
  }

  if (rhs->rtype == RTYPE_MATR) {
    _ drop = lhs->elem.matr.matrix;
    lhs->elem.matr = mAdd(&lhs->elem.matr, &rhs->elem.matr);
    free(rhs->elem.matr.matrix);
    return;
  }

  lhs->elem.comp += rhs->elem.comp;
}

void elemSub(elem_t *lhs, elem_t const *rhs) {
  if (lhs->rtype != rhs->rtype) {
    dispErr(__FUNCTION__, "%s", codetomsg(ERR_TYPE_MISMATCH));
    return;
  }

  if (rhs->rtype == RTYPE_MATR) {
    _ drop = lhs->elem.matr.matrix;
    lhs->elem.matr = mSub(&lhs->elem.matr, &rhs->elem.matr);
    return;
  }

  lhs->elem.comp -= rhs->elem.comp;
}

rtype_t elemMul(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_MATR && rhs->rtype == RTYPE_MATR) {
    _ drop = lhs->elem.matr.matrix;
    lhs->elem.matr = mMul(&lhs->elem.matr, &rhs->elem.matr);
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

void elemDiv(elem_t *lhs, elem_t const *rhs) {
  if (lhs->rtype == RTYPE_COMP) lhs->elem.comp /= rhs->elem.comp;
  else smul(&lhs->elem.matr, 1 / rhs->elem.comp);
}

void elemPow(elem_t *lhs, elem_t *rhs) {
  if (lhs->rtype == RTYPE_COMP) {
    lhs->elem.comp = pow(lhs->elem.comp, rhs->elem.comp);
    return;
  }

  if (creal(rhs->elem.comp) < 0) {
    _ drop = lhs->elem.matr.matrix;
    lhs->elem.matr = inverseMatrix(&lhs->elem.matr);
    rhs->elem.comp *= -1;
  }

  unsigned long n = (unsigned long)creal(rhs->elem.comp);
  matrix_t a dropmatr = newMatrix(lhs->elem.matr.rows, lhs->elem.matr.cols);
  memcpy(a.matrix, lhs->elem.matr.matrix, a.rows * a.cols * sizeof(complex));
  for (size_t i = 1; i < n; i++) {
    _ drop = lhs->elem.matr.matrix;
    lhs->elem.matr = mMul(&lhs->elem.matr, &a);
  }
}
