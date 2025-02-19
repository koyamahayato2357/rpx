#include "benchmarking.h"

#ifdef BENCHMARK_MODE
#undef main
int main() {}
#define main main_
#else
bench(example) {
  volatile int a = 0;
  for (int i = 0; i < 100; i++)
    for (int j = 0; j < 100; j++)
      a++;
}
#endif
