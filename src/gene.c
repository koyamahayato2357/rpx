/**
 * @file src/gene.c
 * @brief Define generic functions
 */

#include "gene.h"
#include "mathdef.h"
#include "testing.h"
#include <stdio.h>
#include <string.h>

constexpr double eps = 1e-5;

static bool double_eq(double a, double b) {
  if (fabs(b) < eps) return fabs(a) < eps; // prevent 0-div when b is near 0
  if (a < 0 != b < 0) return false;        // mis signed
  return fabs(a / b - 1.0) < eps;          // cmp based on ratios
}
static bool complex_eq(complex a, complex b) {
  return double_eq(creal(a), creal(b)) && double_eq(cimag(a), cimag(b));
}

test_table(
  double_eq, double_eq, (bool, double, double),
  {
    { true,      1.0,          1.0},
    { true,    1e-10,            0},
    { true,    1e100, 1e100 + 1e10},
    {false,        0,            1},
    {false,      NAN,          NAN},
    {false,      NAN,         1e10},
    {false, INFINITY,     INFINITY},
    {false, INFINITY,        1e100},
}
)
test_table(
  complex_eq, complex_eq, (bool, complex, complex),
  {
    { true,                0,          0},
    { true,           1 + 3i, 1.0 + 3.0i},
    { true, 1.0 + 1e-10 + 3i,     1 + 3i},
    {false,                0,          1},
    {false,           3 + 5i,    -1 + 3i},
}
)

#pragma clang attribute push(overloadable, apply_to = function)
void printany(int x) {
  printf("%d", x);
}
void printany(size_t x) {
  printf("%zu", x);
}
void printany(double x) {
  printf("%lf", x);
}
void printany(char const *x) {
  printf("%s", x);
}
void printany(char x) {
  printf("%c", x);
}
void printany(bool x) {
  printf(x ? "`true`" : "`false`");
}
void printany(long x) {
  printf("%ld", x);
}
void printany(long long x) {
  printf("%lld", x);
}
void printany(void *x) {
  printf("0x%p", x);
}

#define EQ_DIRECTLY(T) \
  bool eq(T x, T y) { \
    return x == y; \
  }
EQ_DIRECTLY(int)
EQ_DIRECTLY(size_t)
EQ_DIRECTLY(char)
EQ_DIRECTLY(bool)
EQ_DIRECTLY(void *)

bool eq(double x, double y) {
  return double_eq(x, y);
}
bool eq(complex x, complex y) {
  return complex_eq(x, y);
}
bool eq(char const *x, char const *y) {
  return !strcmp(x, y);
}
#pragma clang attribute pop
