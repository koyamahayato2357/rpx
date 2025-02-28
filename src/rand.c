#include "rand.h"
#include <stdarg.h>
#include <time.h>

uint64_t xorsh(rand_method_t met, ...) {
  static uint64_t state;
  if (met == GEN) {
    // generate rand
    uint64_t x = state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state = x;
  }
  // set seed
  va_list ap;
  va_start(ap, met);
  state = va_arg(ap, uint64_t);
  va_end(ap);
  return 0;
}

double xorsh_0_1() {
  return (double)xorsh(GEN) / (double)~(uint64_t)0;
}

[[gnu::constructor]] void __init_xorsh() {
  xorsh(SET, (uint64_t)time(nullptr));
  xorsh(SET, xorsh(GEN));
}
