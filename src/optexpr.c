/**
 * @file src/optexpr.c
 */

#include "optexpr.h"
#include "chore.h"
#include "testing.h"
#include <ctype.h>
#include <string.h>

static void rmExprSpaces(char **expr) {
  char const *start = *expr;
  for (; **expr != '\0'; (*expr)++) {
    if (isspace(**expr)) { // skip unnecessary spaces
      char *dstptr = *expr;
      skipSpaces((char const **)expr);
      dstptr += dstptr != start && isdigit(*(dstptr - 1)) && isdigit(**expr);
      memmove(dstptr, *expr, strlen(*expr) + 1);
      *expr = dstptr;
    }
  }
}

void optexpr(char *arg_expr) {
  char *expr = arg_expr;
  rmExprSpaces(&expr);
}

test (optexpr) {
  char str1[] = "4   5   6   +";
  optexpr(str1);
  expecteq("4 5 6+", (char *)str1);
  char str2[] = "(1 s 2 ^) (1 c 2 ^) +";
  optexpr(str2);
  expecteq("(1s2^)(1c2^)+", (char *)str2);
}
