/**
 * @file src/rand.c
 * @brief Define xorshift
 */

#include "rand.h"
#include "benchmarking.h"
#include "mathdef.h"
#include "testing.h"
#include <stdarg.h>
#include <stdio.h>
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

double xorsh0to1() {
  return (double)xorsh() / (double)~(uint64_t)0;
}

void sxorsh(uint64_t s) {
  state = s;
}

[[gnu::constructor(101)]] void initXorsh() {
  sxorsh((uint64_t)clock());
  _ = xorsh();
}

test (rand) {
  expect(true);

  test_filter("all,rand") {
    constexpr size_t n = 10'000;
    srand((unsigned)clock());

    unsigned rand_arr[n];
    unsigned xors_arr[n];
    double rand_av = 0;
    double xors_av = 0;
    for (size_t i = 0; i < n; i++) {
      rand_arr[i] = (unsigned _BitInt(31))rand();
      xors_arr[i] = (unsigned _BitInt(31))xorsh();
      rand_av += rand_arr[i] / n;
      xors_av += xors_arr[i] / n;
    }
    double rand_vari = 0;
    double xors_vari = 0;
    for (size_t i = 0; i < n; i++) {
      rand_vari += pow(rand_arr[i] - rand_av, 2) / n;
      xors_vari += pow(xors_arr[i] - xors_av, 2) / n;
    }
    puts("");
    printf("rand: av %lf vari %lf\n", rand_av, rand_vari);
    printf("xors: av %lf vari %lf\n", xors_av, xors_vari);
  }
}

bench (rand) { // 0.83 ms
  _ = rand();
}

bench (xorsh) { // 0.81 ms
  _ = xorsh();
}

bench_cycle(rand) {
  _ = rand();
}

bench_cycle(xors) {
  _ = xorsh();
}
