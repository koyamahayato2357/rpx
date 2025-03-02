/**
 * @file include/mathdef.h
 * @brief Wrapper for tgmath.h
 */

#include <tgmath.h>
#undef complex
typedef double _Complex complex;
