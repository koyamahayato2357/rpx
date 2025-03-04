/**
 * @file src/error.c
 */

#include "chore.h"
#include <stdarg.h>

overloadable void
disperr(FILE *fp, char const *funcname, char const *errmsgfmt, ...) {
  va_list ap;
  va_start(ap);

#ifdef TEST_MODE
  _ = errmsgfmt;
  _ = funcname;
  _ = fp;
#else
 #include "ansiesc.h"
 #include <stdio.h>
  fprintf(fp, "\n[%s]\n", funcname);
  fprintf(fp, "  " ESCRED "Error: ");
  vfprintf(fp, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);
#endif

  va_end(ap);
}

overloadable void disperr(char const *funcname, char const *errmsgfmt, ...) {
  va_list ap;
  va_start(ap);
  disperr(stderr, funcname, errmsgfmt, ap);
  va_end(ap);
}
