#include "chore.h"
#include "exproriented.h"
#include "testing.h"
#include <ctype.h>
#include <string.h>
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
inline bool isint(double arg) {
  // forced to implement this way to suppress type warnings
  long unsigned x = *(long unsigned *)&arg;
  long unsigned mantissa = (x & 0x000fffffffffffff);
  short unsigned expo = (x >> 52) & 0x7ff;
  return expo > 1023 && !((mantissa << (expo - 1023)) & 0x000fffffffffffff);
}

/**
 * @brief Skip pointer to first non-white-space char
 */
[[gnu::nonnull]] void skipspcs(char const **restrict str) {
  for (; isspace(**str); (*str)++)
    ;
}

/**
 * @brief Skip pointer to the char following the comma
 * @param[in,out] s String pointer
 */
[[gnu::nonnull]] void skip_untilcomma(char const **restrict s) {
  *s = strchr(*s, ',') ?: *s + strlen(*s);
  *s += !!**s;
}

/**
 * @brief panic alloc
 * @param[in] sz Memory size
 */
[[nodiscard, gnu::returns_nonnull]] void *palloc(size_t sz) {
  return malloc(sz) ?: p$panic(ERR_ALLOCATION_FAILURE);
}

/**
 * @brief free for drop
 */
[[gnu::nonnull]] void free_cl(void *p) { free(*(void **)p); }
[[gnu::nonnull]] void fclose_cl(FILE **fp) { fclose(*fp); }
[[gnu::nonnull]] void closedir_cl(DIR **fp) { closedir(*fp); }

test_table(isint, isint, (bool, double),
           {{true, 5.0},
            {true, 3.000},
            {true, 100},
            {true, -10},
            {false, 5.6},
            {false, 10.9},
            {false, 99.99999},
            {false, -10.4}})
