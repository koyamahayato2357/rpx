#include "rand.h"
#include "benchmarking.h"
#include "mathdef.h"
#include "testing.h"
#include <stdarg.h>
#include <stdlib.h>
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

test (rand) {
  test_filter("rand") {
    srand((unsigned)clock());

    unsigned rand_arr[10'000];
    unsigned xors_arr[10'000];
    double rand_av = 0;
    double xors_av = 0;
    for (int i = 0; i < 10'000; i++) {
      rand_arr[i] = (unsigned)rand();
      xors_arr[i] = (unsigned)xorsh();
      rand_av += rand_arr[i] / 10'000;
      xors_av += xors_arr[i] / 10'000;
    }
    double rand_vari = 0;
    double xors_vari = 0;
    for (int i = 0; i < 10'000; i++) {
      rand_vari += pow(rand_arr[i] - rand_av, 2) / 10'000;
      xors_vari += pow(xors_arr[i] - xors_av, 2) / 10'000;
    }
    printf("rand: av %lf vari %lf\n", rand_av, rand_vari);
    printf("xors: av %lf vari %lf\n", xors_av, xors_vari);
  }
}

bench (rand) { // 0.83 ms
  rand();
}

bench (xorsh) { // 0.81 ms
  xorsh();
}
