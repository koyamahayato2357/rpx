#pragma once
#include "chore.h"
#include <stddef.h>

#define DEF_GEN(T)                                                             \
  void printany(T) overloadable;                                               \
  bool eq(T, T) overloadable;

#define APPLY_PRIMITIVE_TYPES(M) M(int) M(size_t) M(double) M(char) M(bool)
#define APPLY_POINTER_TYPES(M)                                                 \
  M(int *) M(size_t *) M(double *) M(char *) M(bool *) M(void *)

APPLY_PRIMITIVE_TYPES(DEF_GEN)
APPLY_POINTER_TYPES(DEF_GEN)
