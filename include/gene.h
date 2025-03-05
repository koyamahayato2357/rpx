/**
 * @file include/gene.h
 * @brief Define macros for generic programming
 */

#pragma once
#include "def.h"
#include <stddef.h>

#define overloadable [[clang::overloadable]]

#define DEF_GEN(T) \
  void printany(T); \
  bool eq(T, T);

#define APPLY_PRIMITIVE_TYPES(M) \
  M(bool) M(char) M(int) M(size_t) M(long) M(long long) M(double)
#define APPLY_POINTER_TYPES(M) \
  M(bool *) \
  M(char const *) \
  M(int *) M(size_t *) M(void *) M(long *) M(long long *) M(double *)
#define APPLY_ADDSUB(M) M(add, +) M(sub, -)
#define APPLY_ARTHM(M)  APPLY_ADDSUB(M) M(mul, *) M(div, /)
#define APPLY_LTGT(M)   M(lt, <) M(gt, >)

#pragma clang attribute push(overloadable, apply_to = function)
APPLY_PRIMITIVE_TYPES(DEF_GEN)
APPLY_POINTER_TYPES(DEF_GEN)
#pragma clang attribute pop

#define _PRINTREC1(first, ...) \
  printany(first); \
  __VA_OPT__(DEFER(_PRINTREC2)()(__VA_ARGS__))
#define _PRINTREC2() _PRINTREC1
#define PRINT(...) \
  do { EVAL(_PRINTREC1(__VA_ARGS__)) } while (0)
