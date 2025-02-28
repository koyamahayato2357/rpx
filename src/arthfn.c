#include "arthfn.h"
#include "mathdef.h"
#include "testing.h"

double gcd(double n0, double m0) {
  unsigned long n = (unsigned long)n0;
  unsigned long m = (unsigned long)m0;
  while (m != 0) {
    unsigned long temp = n;
    n = m;
    m = temp % m;
  }
  return (double)n;
}

test_table(
  gcd, gcd, (double, double, double),
  {
    { 6,  12, 18},
    { 5,  30, 25},
    {10, 110, 90},
}
)

double lcm(double n, double m) {
  return n / gcd(n, m) * m;
}

test_table(
  lcm, lcm, (double, double, double),
  {
    { 36,  12, 18},
    {150,  30, 25},
    {990, 110, 90},
}
)

double permutation(double n, double r) {
  return tgamma(n + 1) / tgamma(n - r + 1);
}

double combination(double n, double r) {
  return tgamma(n + 1) / tgamma(r + 1) / tgamma(n - r + 1);
}
