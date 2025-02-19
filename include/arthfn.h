#pragma once

#define SNAN __builtin_nans("")
#define M_PI 3.14159265358979323846
#define M_E 2.7182818284590452354

[[gnu::const]] double gcd(double, double);
[[gnu::const]] double lcm(double, double);
[[gnu::const]] double permutation(double, double);
[[gnu::const]] double combination(double, double);
