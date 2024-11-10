#include "optexpr.h"
#include "chore.h"
#include "main.h"
#include <ctype.h>
#include <string.h>

void rm_exprspcs(char **expr) {
  char *start = *expr;
  for (; **expr != '\0'; (*expr)++) {
    if (isspace(**expr)) { // skip unnecessary spaces
      char *dstptr = *expr;
      skipspcs(expr);
      dstptr += dstptr != start && isdigit(*(dstptr - 1)) && isdigit(**expr);
      memmove(dstptr, *expr, strlen(*expr) + 1);
      *expr = dstptr;
    }
  }
}

void optexpr(char *arg_expr) {
  char *expr = arg_expr;
  rm_exprspcs(&expr);

  // orz
  //   expr = arg_expr;
  //   char *grpbgn = expr;
  //   bool isvar1st = false;
  //   for (; *expr != '\0'; expr++) {
  //     if (*expr == '$') {
  //       expr++;
  //       if (isdigit(*expr)) {
  //         char vname = *expr;
  //         if (grpbgn == expr - 1) {
  //           isvar1st = true;
  //           grpbgn = expr + 1;
  //         } else
  //           *(expr - 1) = *expr = ' ';
  //
  //         for (;; expr++) {
  //           if (!is_op(*expr) && *expr != '\0')
  //             continue;
  //           char temp = *(expr + 1);
  //           *(expr + 1) = '\0';
  //           double res = eval_expr_real(grpbgn).elem.real;
  //           *(expr + 1) = temp;
  //           int len = expr - grpbgn;
  //           int n = snprintf(grpbgn, len, "%lf", res);
  //           if (n > len) {
  //             strcpy(expr + n - len, expr);
  //             snprintf(grpbgn, n, "%lf", res);
  //           }
  //           grpbgn += n;
  //           if (!isvar1st) {
  //             if (n >= len)
  //               strcpy(grpbgn + 2, grpbgn);
  //             *grpbgn = '$';
  //             *(grpbgn + 1) = vname;
  //             grpbgn += 2;
  //           }
  //           if (n < len)
  //             memmove(grpbgn, expr, len + 1);
  //           grpbgn = expr;
  //           goto end;
  //         }
  //       }
  //     }
  //   }
  // end:
  // for (; *expr != '\0';) {
  //   if (*++expr == '$') {
  //     grpbgn = expr;
  //     expr += !islower(*expr);
  //   }
  //
  //   buf[bufidx++] = *expr;
  //   if (is_op(*expr)) {
  //     buf[bufidx] = '\0';
  //     double res = eval_f(buf).real;
  //     // memcpy(grpbgn, res.to_str())
  //   }
  //   expr++;
  // }
}
