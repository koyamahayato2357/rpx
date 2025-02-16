#include "ansiesc.h"
#include "chore.h"
#include <stdarg.h>
#include <stdio.h>

__attribute__((format(printf, 2, 3))) void disperr(char const *funcname,
                                                   char const *errmsgfmt, ...) {
#ifndef TEST_MODE
  va_list ap;
  va_start(ap, errmsgfmt);

  // to remove warning that va_list argument is uninitialized
  _ = va_arg(ap, int);

  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  " ESCRED "Error: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);

  va_end(ap);
#else
  _ = funcname;
  _ = errmsgfmt;
#endif
}
