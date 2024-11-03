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
