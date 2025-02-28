#pragma once
#include <stdint.h>

typedef enum {
  SET,
  GEN,
} rand_method_t;

typedef uint64_t rand_fn_t(rand_method_t, ...);

rand_fn_t xorsh;
double xorsh_0_1();
