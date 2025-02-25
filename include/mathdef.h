#include <tgmath.h>
#undef complex
#define isnan(x) __builtin_isnan(x)
typedef double _Complex complex;
