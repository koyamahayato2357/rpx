#include "ansiesc.h"
#include <stdarg.h>
#include <stdio.h>

#ifndef TEST_MODE
void disperr(const char *funcname, const char *errmsgfmt, ...) {
  va_list ap;
  va_start(ap, errmsgfmt);

  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  " ESCRED "Error: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);

  va_end(ap);
}
#endif
