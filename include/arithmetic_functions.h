#pragma once

#define SNAN __builtin_nans("")
#define M_PI 3.14159265358979323846
#define M_E 2.7182818284590452354

double gcd(double, double) __attribute__((const));
double lcm(double, double) __attribute__((const));
double permutation(double, double) __attribute__((const));
double combination(double, double) __attribute__((const));
