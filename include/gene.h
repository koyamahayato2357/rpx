/**
 * @file include/gene.h
 * @brief Define macros for generic programming
 */

#pragma once
#include "def.h"
#include <stddef.h>

#define overloadable [[clang::overloadable]]

#define DEF_PRIM(T) \
  overloadable void printany(T); \
  overloadable bool eq(T, T);
#define DEF_GEN(T) DEF_PRIM(T) DEF_PRIM(T *)

#define TYPES bool, char, int, size_t, long, long long, double

#define APPLY_TYPES(M) MAP(M, TYPES)

#define APPLY_ADDSUB(M) M(add, +) M(sub, -)
#define APPLY_ARTHM(M)  APPLY_ADDSUB(M) M(mul, *) M(div, /)
#define APPLY_LTGT(M)   M(lt, <) M(gt, >)

APPLY_TYPES(DEF_GEN)

#define _PRINTREC1(first, ...) \
  printany(first); \
  __VA_OPT__(DEFER(_PRINTREC2)()(__VA_ARGS__))
#define _PRINTREC2() _PRINTREC1
#define PRINT(...) \
  do { EVAL(_PRINTREC1(__VA_ARGS__)) } while (0)
