#include "testing.h"

#ifdef TEST_MODE
#undef main
int main() {}
#define main main_
int TESTING_H_success;
int TESTING_H_fail;
__attribute__((destructor)) void TESTING_H_report_test_result() {
  printf("\n" ESCBLU "Passed" ESCLR ": %d/%d\n", TESTING_H_success,
         TESTING_H_success + TESTING_H_fail);
}
#endif

/* usage */
/*

double add(int a, int b) {
  return (a + b) / 2.0;
}

test(add_test) {
  expect(add(1, 1) == 1.0);
  expecteq(2.0, add(3, 1));
  expectneq(8, add(3, 9));
}

test_table(add_test_table, add, (double, int, int),
           {{ 2.5, 3, 2 },
            { 3.5, 3, 4 },
            { 4.5, 1, 8 }})

*/
