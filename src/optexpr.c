#include "optexpr.h"
#include "chore.h"
#include <ctype.h>
#include <string.h>

void rm_exprspcs(char **expr) {
  char *start = *expr;
  for (; **expr != '\0'; (*expr)++) {
    if (isspace(**expr)) { // skip unnecessary spaces
      char *dstptr = *expr;
      skipspcs((char const **)expr);
      dstptr += dstptr != start && isdigit(*(dstptr - 1)) && isdigit(**expr);
      memmove(dstptr, *expr, strlen(*expr) + 1);
      *expr = dstptr;
    }
  }
}

void optexpr(char *arg_expr) {
  char *expr = arg_expr;
  rm_exprspcs(&expr);
}
