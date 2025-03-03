/**
 * @file src/benchmarking.c
 */

#ifdef BENCHMARK_MODE
int main() {
}
#else
 #include "benchmarking.h"

bench (example) {
  int volatile a = 0;
  for (int i = 0; i < 100; i++)
    for (int j = 0; j < 100; j++) a++;
}
#endif
