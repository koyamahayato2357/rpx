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

overloadable void printany(int x) {
  printf("%d", x);
}
overloadable void printany(size_t x) {
  printf("%zu", x);
}
overloadable void printany(double x) {
  printf("%lf", x);
}
overloadable void printany(char x) {
  printf("'%c'", x);
}
overloadable void printany(bool x) {
  printf(x ? "true" : "false");
}
overloadable void printany(char *x) {
  printf("\"%s\"", x);
}
overloadable void printany(void *x) {
  printf("%p", x);
}

overloadable bool eq(int x, int y) {
  return x == y;
}
overloadable bool eq(size_t x, size_t y) {
  return x == y;
}
overloadable bool eq(double x, double y) {
  return double_eq(x, y);
}
overloadable bool eq(char x, char y) {
  return complex_eq(x, y);
}
overloadable bool eq(bool x, bool y) {
  return x == y;
}
overloadable bool eq(char *x, char *y) {
  return !strcmp(x, y);
}
overloadable bool eq(void *x, void *y) {
  return x == y;
}
