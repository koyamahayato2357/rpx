/**
 * @file src/chore.c
 * @brief Define util functions
 */

#include "chore.h"
#include "exproriented.h"
#include "mathdef.h"
#include "testing.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * @brief Get terminal window size
 * @return Struct in which window size if stored
 */
struct winsize getWinSize() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w;
}

overloadable bool eq(struct winsize l, struct winsize r) {
  return l.ws_col == r.ws_col && l.ws_row == r.ws_row
      && l.ws_xpixel == r.ws_xpixel && l.ws_ypixel == r.ws_ypixel;
}
overloadable void printany(struct winsize ws) {
  _ = ws;
}
#define ST_WS(r, c, x, y) \
  (struct winsize) { \
    .ws_row = r, .ws_col = c, .ws_xpixel = x, .ws_ypixel = y \
  }

test (get_winsz) { expectneq(ST_WS(0, 0, 0, 0), getWinSize()); }

/**
 * @brief Check if the decimal part is 0
 * @param[in] arg Checked double number
 * @return Is decimal part 0
 */
inline bool isInt(double arg) {
  if (isinf(arg) || isnan(arg)) return false;
  if (arg < LONG_MIN || (double)LONG_MAX < arg) return false;
  return arg == (double)(long)arg;
}

test_table(
  isint, isInt, (bool, double),
  {
    { true,      5.0},
    { true,    3.000},
    { true,      100},
    { true,      -10},
    {false,      5.6},
    {false,     10.9},
    {false, 99.99999},
    {false,    -10.4}
}
)

/**
 * @brief Skip pointer to first non-white-space char
 */
overloadable void skipSpaces(char const **restrict str) {
  [[clang::always_inline]] skipSpaces(str, ~0UL);
}
overloadable void skipSpaces(char const **restrict str, size_t len) {
  for (size_t i = 0; i < len && isspace(**str); i++, (*str)++);
}

/**
 * @brief Skip pointer to the char following the comma
 * @param[in,out] s String pointer
 */
void skipUntilComma(char const **restrict s) {
  *s = strchr(*s, ',') orelse * s + strlen(*s) - 1;
  (*s)++;
}

/**
 * @brief panic alloc
 * @param[in] sz Memory size
 * @warning Unrecoverable
 */
void *palloc(size_t sz) {
  return malloc(sz) orelse p$panic(ERR_ALLOCATION_FAILURE);
}

/**
 * @brief free for drop
 */
void freecl(void *p) {
  free(*(void **)p);
}
void fclosecl(FILE **fp) {
  fclose(*fp);
}
void closedircl(DIR **fp) {
  closedir(*fp);
}
