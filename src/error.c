#include "ansiesc.h"
#include "chore.h"
#include <stdarg.h>
#include <stdio.h>

[[gnu::format(printf, 2, 3)]] void disperr(char const *funcname,
                                           char const *errmsgfmt, ...) {
  va_list ap;
  va_start(ap, errmsgfmt);

  // to remove warning that va_list argument is uninitialized
  _ = va_arg(ap, int);

#ifdef TEST_MODE
  _ = funcname;
  _ = errmsgfmt;
#else
  fprintf(stderr, "\n[%s]\n", funcname);
  fprintf(stderr, "  " ESCRED "Error: ");
  vfprintf(stderr, errmsgfmt, ap);
  fputs(ESCLR "\n", stderr);

  va_end(ap);
#endif
}
