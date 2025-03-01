#pragma once
#include <stdint.h>

typedef enum {
  SET,
  GEN,
} rand_method_t;

uint64_t xorsh();
double xorsh_0_1();
void sxorsh(uint64_t);
