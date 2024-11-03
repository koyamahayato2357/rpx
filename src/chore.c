#include "chore.h"
#include "errcode.h"
#include "exception.h"
#include "exproriented.h"
#include "testing.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * @brief Get terminal window size
 * @return Struct in which window size if stored
 */
struct winsize get_winsz() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w;
}

/**
 * @brief Check if the decimal part is 0
 * @param[in] arg Checked double number
 * @return Is decimal part 0
 */
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

/**
 * @brief Skip pointer to first non-white-space char
 */
void skipspcs(char **str) {
  for (; isspace(**str); (*str)++)
    ;
}

/**
 * @brief Skip pointer to the char following the comma
 * @param[in/out] s String pointer
 */
void skip_untilcomma(char **s) {
  for (; **s != ',' && **s != '\0'; (*s)++)
    ;
  *s += **s == ',';
}

/**
 * @brief Nullable free
 * @param[in] p Nullable pointer
 */
void nfree(void *p) {
  if (p)
    free(p);
}

/**
 * @brief Exception alloc
 * @param[in] sz Memory size
 * @throws ERR_ALLOCATION_FAILURE
 */
void *ealloc(int sz) { return malloc(sz) ?: p$throw(ERR_ALLOCATION_FAILURE); }

/**
 * @brief panic alloc
 * @param[in] sz Memory size
 */
void *palloc(int sz) { return malloc(sz) ?: p$panic(ERR_ALLOCATION_FAILURE); }
