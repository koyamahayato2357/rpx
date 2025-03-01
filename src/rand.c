#include "rand.h"
#include <stdarg.h>
#include <time.h>

thread_local uint64_t state;

uint64_t xorsh() {
    uint64_t x = state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state = x;
}

double xorsh_0_1() {
  return (double)xorsh() / (double)~(uint64_t)0;
}

void sxorsh(uint64_t s) {
  state = s;
}

[[gnu::constructor(101)]] void __init_xorsh() {
  sxorsh((uint64_t)clock());
  xorsh();
}
