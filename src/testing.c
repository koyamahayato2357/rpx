/**
 * @file src/testing.c
 */

#ifdef TEST_MODE
int TEST_success;
int TEST_count;
int main() {
  return TEST_count - TEST_success;
}
 #include "testing.h"
[[gnu::destructor]] void TEST_report_test_result() {
  printf("\n" ESCBLU "Passed" ESCLR ": %d/%d\n", TEST_success, TEST_count);
}

test (testing_test) {
  expect(1 + 1 == 2);
  expecteq(3, 3);
  expectneq(1, 10);

  test_filter("testing test") {
    expect(false);
    expect(1 + 1 == 1);

    expecteq(5, 3);
    expecteq(3, 10);

    expectneq(10, 10);

    testing_unreachable;
  }
}

#endif
