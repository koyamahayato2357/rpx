/**
 * @file include/mathdef.h
 * @brief Wrapper for tgmath.h
 */

#pragma once
#include <tgmath.h>
#undef complex
typedef double _Complex complex;

constexpr double M_PI = 3.14159265358979323846;
constexpr double M_E = 2.7182818284590452354;
