#include "arthfn.h"
#include <tgmath.h>

double gcd(double n, double m) {
  while (m) {
    double temp = n;
    n = m;
    m = fmod(temp, m);
  }
  return fabs(n);
}

double lcm(double n, double m) { return fabs(n / gcd(n, m) * m); }

double permutation(double n, double r) {
  return tgamma(n + 1) / tgamma(n - r + 1);
}

double combination(double n, double r) {
  return tgamma(n + 1) / tgamma(r + 1) / tgamma(n - r + 1);
}
