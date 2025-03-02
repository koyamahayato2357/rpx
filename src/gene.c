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

#pragma clang attribute push(overloadable, apply_to = function)
void printany(int x) {
  printf("%d", x);
}
void printany(size_t x) {
  printf("%zuUL", x);
}
void printany(double x) {
  printf("%lf", x);
}
void printany(char x) {
  printf("'%c'", x);
}
void printany(bool x) {
  printf(x ? "true" : "false");
}
void printany(char *x) {
  printf("\"%s\"", x);
}
void printany(void *x) {
  printf("0x%p", x);
}

bool eq(int x, int y) {
  return x == y;
}
bool eq(size_t x, size_t y) {
  return x == y;
}
bool eq(double x, double y) {
  return double_eq(x, y);
}
bool eq(char x, char y) {
  return complex_eq(x, y);
}
bool eq(bool x, bool y) {
  return x == y;
}
bool eq(char *x, char *y) {
  return !strcmp(x, y);
}
bool eq(void *x, void *y) {
  return x == y;
}
#pragma clang attribute pop
