#include "testing.h"

#ifdef TEST_MODE
#undef main
int main() {}
#define main main_
#endif

void _expect(bool cond, unsigned int line) {
  if (!cond) {
    fprintf(stderr, "Unexpected cond in line %d\n", line);
    throw(1);
  }
}
bool double_eq(double a, double b) { return fabs(a - b) < EPSILON; }
bool complex_eq(double complex a, double complex b) {
  return fabs(creal(a - b)) < EPSILON && fabs(cimag(a - b)) < EPSILON;
}
