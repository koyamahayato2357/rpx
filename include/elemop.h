/**
 * @file include/elemop.h
 */

#include "main.h"

void elemSet(elem_t *, elem_t const *);
bool elemEq(elem_t const *, elem_t const *);
void elemAdd(elem_t *, elem_t const *);
void elemSub(elem_t *, elem_t const *);
rtype_t elemMul(elem_t *, elem_t *);
void elemDiv(elem_t *, elem_t const *);
void elemPow(elem_t *, elem_t *);
