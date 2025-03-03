/**
 * @file src/testing.c
 */

#include "testing.h"

#ifdef TEST_MODE
 #undef main
int main() {
}
 #define main main_
int TEST_success;
int TEST_count;
[[gnu::destructor]] void TEST_report_test_result() {
  printf("\n" ESCBLU "Passed" ESCLR ": %d/%d\n", TEST_success, TEST_count);
}

test(name) {
  test_filter("testing test") {
    expect(false);
    expect(1 + 1 == 1);
    expect(1 + 1 == 2);

    expecteq(3, 3);
    expecteq(5, 3);
    expecteq(3, 10);

    expectneq(1, 10);
    expectneq(10, 10);

    testing_unreachable;
  }
}

#endif
