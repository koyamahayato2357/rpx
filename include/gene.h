#pragma once
#include <stddef.h>

#define overloadable [[clang::overloadable]]
#define ASSUME_OVERLOADABLE_BGN \
  _Pragma("clang attribute push(overloadable, apply_to = function)")

#define ASSUME_OVERLOADABLE_END \
  _Pragma("clang attribute pop(overloadable, apply_to = function)")

#define DEF_GEN(T) \
  void printany(T); \
  bool eq(T, T);

#define APPLY_PRIMITIVE_TYPES(M) M(int) M(size_t) M(double) M(char) M(bool)
#define APPLY_POINTER_TYPES(M) \
  M(int *) M(size_t *) M(double *) M(char *) M(bool *) M(void *)
#define APPLY_ADDSUB(M) M(add, +) M(sub, -)
#define APPLY_ARTHM(M)  APPLY_ADDSUB(M) M(mul, *) M(div, /)
#define APPLY_LTGT(M)   M(lt, <) M(gt, >)

#pragma clang attribute push(overloadable, apply_to = function)
APPLY_PRIMITIVE_TYPES(DEF_GEN)
APPLY_POINTER_TYPES(DEF_GEN)
#pragma clang attribute pop
