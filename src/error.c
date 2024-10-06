#include <stdarg.h>
#include <stdio.h>

#ifndef TEST_MODE
void disperr(const char *funcname, const char *errmsgfmt, ...) {
  va_list ap;
  va_start(ap, errmsgfmt);

  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  \033[31mError: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs("\033[0m\n", stderr);

  va_end(ap);
}
#endif
