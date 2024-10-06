#include "chore.h"
#include "errcode.h"
#include "exception.h"
#include "testing.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

struct winsize get_winsz() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w;
}

bool isint(double arg) {
  uint64_t x = *(uint64_t *)&arg;
  uint64_t mantissa = (x & 0x000fffffffffffff);
  uint16_t expo = (x >> 52) & 0x7ff;

  return expo > 1023 && !((mantissa << (expo - 1023)) & 0x000fffffffffffff);
}

test(isint) {
  expect(isint(5.0));
  expect(isint(3.000));
  expect(isint(100));
  expect(isint(-10));
  expect(!isint(5.6));
  expect(!isint(10.9));
  expect(!isint(99.99999));
  expect(!isint(-10.4));
}

void skipspcs(char **str) {
  for (; isspace(**str); (*str)++)
    ;
}

void skip_untilcomma(char **s) {
  for (; **s != ',' && **s != '\0'; (*s)++)
    ;
  *s += **s == ',';
}

void nfree(void *p) {
  if (p)
    free(p);
}

/**
 * @brief Exception alloc
 * @param[in] sz Memory size
 * @throws ERR_ALLOCATION_FAILURE
 */
void *ealloc(int sz) {
  void *mem = malloc(sz);
  if (mem == nullptr)
    throw(ERR_ALLOCATION_FAILURE);
  return mem;
}
