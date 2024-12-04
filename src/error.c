#include "ansiesc.h"
#include "chore.h"
#include <stdarg.h>
#include <stdio.h>

#ifndef TEST_MODE
void disperr(char const *funcname, char const *errmsgfmt, ...) {
  va_list ap;
  va_start(ap, errmsgfmt);

  // to remove warning that va_list argument is uninitialized
  _ = va_arg(ap, int);

  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  " ESCRED "Error: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);

  va_end(ap);
}
#endif
