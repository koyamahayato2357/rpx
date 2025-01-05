#include "chore.h"
#include "exproriented.h"
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
inline bool isint(double arg) { return arg == (long long)arg; }

/**
 * @brief Skip pointer to first non-white-space char
 */
void skipspcs(char const **str) {
  for (; isspace(**str); (*str)++)
    ;
}

/**
 * @brief Skip pointer to the char following the comma
 * @param[in/out] s String pointer
 */
void skip_untilcomma(char const **s) {
  *s = strchr(*s, ',') ?: *s + strlen(*s);
  *s += !!**s;
}

/**
 * @brief Nullable free
 * @param[in] p Nullable pointer
 */
inline void nfree(void *p) {
  if (p)
    free(p);
}

/**
 * @brief panic alloc
 * @param[in] sz Memory size
 */
[[nodiscard]] void *palloc(int sz) {
  return malloc(sz) ?: p$panic(ERR_ALLOCATION_FAILURE);
}

/**
 * @brief free for drop
 */
void free_cl(void *p) { free(*(void **)p); }
void fclose_cl(FILE **fp) { fclose(*fp); }
void closedir_cl(DIR **fp) { closedir(*fp); }
