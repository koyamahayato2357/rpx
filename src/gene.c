#include "gene.h"
#include "mathdef.h"
#include <stdio.h>
#include <string.h>

constexpr double eps = 1e-5;

static bool double_eq(double a, double b) {
  return fabs(a - b) < eps;
}
static bool complex_eq(complex a, complex b) {
  return fabs(creal(a - b)) < eps && fabs(cimag(a - b)) < eps;
}

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
  printf("%s", x);
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
