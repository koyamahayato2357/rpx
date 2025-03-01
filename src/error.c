#include "chore.h"
#include <stdarg.h>

[[gnu::format(printf, 2, 3)]] void
disperr(char const *funcname, char const *errmsgfmt, ...) {
  va_list ap;
  va_start(ap);

#ifdef TEST_MODE
  _ = errmsgfmt;
  _ = funcname;
#else
 #include "ansiesc.h"
 #include <stdio.h>
  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  " ESCRED "Error: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);
#endif

  va_end(ap);
}
