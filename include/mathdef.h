/**
 * @file include/mathdef.h
 * @brief Wrapper for tgmath.h
 */

#pragma once
#include <tgmath.h>
#undef complex
typedef double _Complex complex;

constexpr double pi = 3.14159265358979323846;
constexpr double euler = 2.7182818284590452354;
