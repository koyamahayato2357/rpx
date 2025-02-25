#include <stdio.h>

void putsequence(char *seq) {
  fputs(seq, stdout);
}

#ifdef TEST_MODE_ALL
 #include "ansiesc.h"
 #include "testing.h"

test (ansi_escape_sequence) {
  puts(ESCLR);

  puts(ESBLD "hello" ESCLR);
  puts(ESTHN "hello" ESCLR);
  puts(ESITA "hello" ESCLR);
  puts(ESULN "hello" ESCLR);
  puts(ESBLN "hello" ESCLR);
  puts(ESFBLN "hello" ESCLR);
  puts(ESREV "hello" ESCLR);
  puts(ESHID "hello" ESCLR);
  puts(ESUDO "hello" ESCLR);

  puts(ESCRED "hello" ESCLR);
  puts(ESCGRN "hello" ESCLR);
  puts(ESCYEL "hello" ESCLR);
  puts(ESCBLU "hello" ESCLR);
  puts(ESCMGN "hello" ESCLR);
  puts(ESCCYN "hello" ESCLR);
  puts(ESCBLK "hello" ESCLR);

  puts(ESCBRED "hello" ESCLR);
  puts(ESCBGRN "hello" ESCLR);
  puts(ESCBYEL "hello" ESCLR);
  puts(ESCBBLU "hello" ESCLR);
  puts(ESCBMGN "hello" ESCLR);
  puts(ESCBCYN "hello" ESCLR);
  puts(ESCBBLK "hello" ESCLR);

  puts(ESCCODE(100) "hello" ESCLR);
  puts(ESCCODE_RGB(100, 100, 100) "hello" ESCLR);
}
#endif
