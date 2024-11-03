/*
 * Usage
 * ----------------------------------------------------------------------------
 * if expression         | puts($if(lhs == rhs) "equal" $else "not equal");
 * ----------------------------------------------------------------------------
 * break expression      | p = next() orelse $break;
 * ----------------------------------------------------------------------------
 * continue expression   | is_validinput(input) orelse $continue;
 * ----------------------------------------------------------------------------
 * return expression     | char *homedir = getenv("HOME") orelse p$return(-1);
 * ----------------------------------------------------------------------------
 */

#include "exproriented.h"
#include "testing.h"

test(in_expr) {
  int i = 0;
  for (;;) {
    int j [[maybe_unused]] = $if(i == 1) 1 $else $break;
    testing_unreachable;
  }
  for (int i = 0; i < 5; i++) {
    int j [[maybe_unused]] = $continue;
    testing_unreachable;
  }
}

test(in_statement) {
  for (;;) {
    $break;
    testing_unreachable;
  }
  for (int i = 0; i < 5; i++) {
    $continue;
    testing_unreachable;
  }
}

test(multi_statement) {
  int a = $if(true)({
    int i = 1;
    i * 2;
  }) $else({
    int i = 5;
    (i + 1) * i;
  });
  expecteq(a, 2);
  a = $if(false)({
    int i = 9;
    i - 1;
  }) $else({
    int i [[maybe_unused]] = a;
    a;
  });
  expecteq(a, 2);
}

test(dollar_sign) {
  // $ expression returns 0.
  // For now.
  // I want Unit type
  int a = $(int i = 5; expecteq(5, i));
  expecteq(0, a);
}
