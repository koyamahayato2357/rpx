/**
 * @file src/editline.c
 * @brief Define `editline()`
 */

#include <stdio.h>
#define __USE_GNU // for memrchr()
#include "ansiesc.h"
#include "chore.h"
#include "editline.h"
#include "error.h"
#include "exproriented.h"
#include "main.h"
#include "testing.h"
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define getchar() ((char)getchar())

// getchar() becomes like getch() in MSVC
static void enableRawMode(struct termios *orig_termios) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, orig_termios);
  raw = *orig_termios;
  raw.c_lflag &= ~(unsigned)(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disableRawMode(struct termios *orig_termios) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

/**
 * @brief Move cursor
 * @param[in] n Number of moves
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 * @param[in] len End of line
 */
static void movecur(int n, char *buf, char **cur, char *len) {
  *cur = $if(n > len - *cur) len $else $if(-n > *cur - buf) buf $else * cur + n;
}

test (movecur) {
  char buf[256] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  movecur(1, buf, &cur, len);
  expecteq(buf + 1, cur);
  movecur(3, buf, &cur, len);
  expecteq(buf + 4, cur);
  movecur(-4, buf, &cur, len);
  expecteq(buf, cur);
  movecur(-1, buf, &cur, len);
  expecteq(buf, cur);
  movecur(999, buf, &cur, len);
  expecteq(len, cur);
}

/**
 * @brief Delete a string from within a string
 * @param[in] begin Beginning of erase range
 * @param[in] end End of erase range
 * @param[in,out] len End of line
 */
[[gnu::nonnull]] static void deletes(char *begin, char const *end, char **len) {
  memcpy(begin, end, (size_t)(*len - end));
  *len -= end - begin;
  if (*len) **len = '\0';
}

test (deletes) {
  char str[] = "sample text.";
  char *len = str + strlen(str);
  char *begin = str;
  char const *end = str + 7;

  deletes(begin, end, &len);
  expecteq("text.", (char *)str);
}

/**
 * @brief Find character and move there
 * @param[in] c Character to look for
 * @param[in] dir Search direction, >0: forward, <=0: backward
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 * @return Found or not
 */
static bool findmove(char c, int dir, char *buf, char **cur) {
  char *old_cur = *cur;
  *cur
    = $if(dir > 0) strchr(*cur, c) $else memrchr(buf, c, (size_t)(*cur - buf));
  if (*cur == nullptr) {
    *cur = old_cur;
    return false;
  }
  return true;
}

test (findmove) {
  char buf[256] = "sample text.";
  char *cur = buf;

  expect(findmove(' ', 1, buf, &cur));
  expecteq(' ', *cur);
  expect(findmove('e', 1, buf, &cur));
  expecteq('e', *cur);
  expect(findmove('s', -1, buf, &cur));
  expecteq('s', *cur);
  expect(!findmove('z', 1, buf, &cur));
  expecteq('s', *cur);
  expect(findmove('.', 1, buf, &cur));
  expecteq('.', *cur);
}

/**
 * @brief Move the cursor forward one word
 * @param[in,out] cur Cursor pointer
 * @param[in] len End of line
 */
static void fwdw(char **cur, char const *len) {
  if (*cur >= len) return;

  int wasalnum = isalnum(**cur);
  int wasspace = isspace(**cur);
  (*cur)++;
  for (; (!isspace(**cur) ^ wasspace) && *cur != len
         && !(isalnum(**cur) ^ wasalnum);
       (*cur)++);

  skipSpaces((char const **)cur);
}

test (fwdw) {
  char buf[] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  fwdw(&cur, len);
  expecteq(buf + 7, cur);
  fwdw(&cur, len);
  expecteq(buf + 11, cur);
}

/**
 * @brief Move the cursor backward one word
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 */
static void bwdw(char const *buf, char **cur) {
  if (buf >= *cur) return;

  (*cur)--;
  for (; isspace(**cur); (*cur)--);

  int wasalnum = isalnum(**cur);
  while (!isspace(*(*cur - 1)) && *cur != buf
         && !(isalnum(*(*cur - 1)) ^ wasalnum))
    (*cur)--;
}

test (bwdw) {
  char buf[] = "sample text.";
  char *cur = buf + strlen(buf);

  bwdw(buf, &cur);
  expecteq(buf + 11, cur);
  bwdw(buf, &cur);
  expecteq(buf + 7, cur);
}

/**
 * @brief Move the cursor forward one WORD
 * @param[in,out] cur Cursor pointer
 * @param[in] len End of line
 */
static void fwdW(char **cur, char *len) {
  *cur = strchr(*cur, ' ') ?: len;
  skipSpaces((char const **)cur);
}

test (fwdW) {
  char buf[] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  fwdW(&cur, len);
  expecteq('t', *cur);
  fwdW(&cur, len);
  expecteq('\0', *cur);
}

/**
 * @brief Move the cursor backward one WORD
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 */
static void bwdW(char *buf, char **cur) {
  if (buf == *cur) return;
  for ((*cur)--; isspace(**cur) && buf < *cur; (*cur)--);
  *cur = memrchr(buf, ' ', (size_t)(*cur - buf)) ?: buf - 1;
  (*cur)++;
}

test (bwdW) {
  char buf[] = "sample text.";
  char *cur = buf + strlen(buf);

  bwdW(buf, &cur);
  expecteq('t', *cur);
  bwdW(buf, &cur);
  expecteq('s', *cur);
}

/**
 * @brief Handle escape sequence
 * @param[in] key Escape sequence
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 * @param[in,out] len End of line
 */
static void handleEs(char key, char *buf, char **cur, char **len) {
  switch (key) {
  case '3': // delete key
    if (getchar() != '~') break;
    if (*len == *cur) break;
    memmove(*cur, *cur + 1, (size_t)(*len - *cur - 1));
    (*len)--;
    break;
  case 'C': // right arrow
    movecur(1, buf, cur, *len);
    break;
  case 'D': // left arrow
    movecur(-1, buf, cur, *len);
    break;
  case 'F': // END
    *cur = *len;
    break;
  case 'H': // HOME
    *cur = buf;
    break;
  default:
    dispErr(__FUNCTION__, "unknown escape sequence: %c", key);
  }
}

/**
 * @brief Handle text object
 * @param[in] txtobj Second character of text object
 * @param[in] buf Start of line
 * @param[in] cur Cursor pointer
 * @param[in] len End of line
 * @param[out] begin Beginning of selected range of text object
 * @param[out] end End of selected range of text object
 */
static void handleTxtObj(
  char txtobj, char *buf, char *cur, char *len, char **begin, char **end
) {
  switch (txtobj) {
  case 'w':
    fwdw(&cur, len);
    *end = cur;
    bwdw(buf, &cur);
    *begin = cur;
    break;
  case 'W':
    fwdW(&cur, len);
    *end = cur;
    bwdW(buf, &cur);
    *begin = cur;
    break;
  case 'b':
    findmove(')', 1, buf, &cur);
    *end = cur;
    findmove('(', -1, buf, &cur);
    *begin = cur + 1;
    break;
  case ']':
    findmove(']', 1, buf, &cur);
    *end = cur;
    findmove('[', -1, buf, &cur);
    *begin = cur + 1;
    break;
  default:
    *begin = nullptr;
    *end = nullptr;
    break;
  }
}

/**
 * @brief Insert a character at cursor position
 * @param[in] c Character to be inserted
 * @param[in,out] cur Cursor pointer
 * @param[in,out] len End of line
 * @details Possible buffer overrun
 */
[[gnu::nonnull]] static void insertc(char c, char **cur, char **len) {
  if (*cur != *len) memmove(*cur + 1, *cur, (size_t)(*len - *cur));
  *(*cur)++ = c;
  *++*len = '\0';
}

/**
 * @brief Insert a string at cursor position
 * @param[in] slen Number of characters in string to be inserted
 * @param[in] s String to be inserted
 * @param[in,out] cur Cursor pointer
 * @param[in,out] len End of line
 * @param[in] margin Number of characters can be added to the buffer
 */
static void
inserts(size_t slen, char const *s, char **cur, char **len, size_t margin) {
  if (slen > margin) {
    dispErr(__FUNCTION__, "buffer depreletion");
    abort();
  }
  if (*cur != *len) memmove(*cur + slen, *cur, (size_t)(*len - *cur));
  memcpy(*cur, s, slen);
  *len += slen;
  **len = '\0';
}

test (inserts) {
  char buf[256] = "sample text.";
  char *len = buf + strlen(buf);
  char *cur = len - 1;

  char const *str = "new string";
  inserts(strlen(str), str, &cur, &len, (size_t)(buf + 256 - len));
  expecteq("sample textnew string.", (char *)buf);
  expecteq("new string.", cur);
  expecteq(strlen(buf), (size_t)(len - buf));

  str = "extra string";
  inserts(strlen(str), str, &cur, &len, (size_t)(buf + 256 - len));
  expecteq("sample textextra stringnew string.", (char *)buf);
  expecteq("extra stringnew string.", cur);
  expecteq(strlen(buf), (size_t)(len - buf));
}

/**
 * @brief Behavior in insert mode
 * @param[in] c Typed character
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 * @param[in,out] len End of line
 */
static void insbind(char c, char *buf, char **cur, char **len) {
  switch (c) {
  case '(': // autopair
    inserts(2, "()", cur, len, (size_t)(buf + buf_size - *len));
    (*cur)++;
    break;

  case ')':
    *cur = 1 + (strchr(*cur, ')') ?: p$(goto dflt));
    break;

  case '[': // autopair
    inserts(2, "[]", cur, len, (size_t)(buf + buf_size - *len));
    (*cur)++;
    break;

  case ']': {
    *cur = 1 + (strchr(*cur, ']') ?: p$(goto dflt));
    break;
  }

dflt:
  default:
    insertc(c, cur, len);
  }
}

auto handle_printable = insbind;

/**
 * @brief Behavior in normal mode
 * @param[in] c Typed character
 * @param[in] buf Start of line
 * @param[in,out] cur Cursor pointer
 * @param[in,out] len End of line
 * @see `$ vim -c ":h *{char}*<CR>"`
 */
static void nrmbind(char c, char *buf, char **cur, char **len) {
  switch (c) {
  case 'h':
    movecur(-1, buf, cur, *len);
    break;
  case 'l':
    movecur(1, buf, cur, *len);
    break;
  case 'w':
    fwdw(cur, *len);
    break;
  case 'b':
    bwdw(buf, cur);
    break;
  case 'W':
    fwdW(cur, *len);
    break;
  case 'B':
    bwdW(buf, cur);
    break;
  case 'f':
    findmove(getchar(), 1, buf, cur);
    break;
  case 'F':
    findmove(getchar(), -1, buf, cur);
    break;
  case 't':
    if (!findmove(getchar(), 1, buf, cur)) break;
    (*cur)--;
    break;
  case 'T':
    if (!findmove(getchar(), -1, buf, cur)) break;
    (*cur)++;
    break;
  case 'A':
    *cur = *len;
    [[fallthrough]];
  case 'a':
    *cur += *cur != *len;
    handle_printable = insbind;
    break;
  case 'I':
    *cur = buf;
    [[fallthrough]];
  case 'i':
    handle_printable = insbind;
    break;
  case 'c':
    handle_printable = insbind;
    [[fallthrough]];
  case 'd': {
    char *end = *cur;
    char input = getchar();
    char *dst, *src;
    if (input == 'i' || input == 'a') {
      handleTxtObj(getchar(), buf, *cur, *len, &dst, &src);
    } else {
      nrmbind(input, buf, &end, len);
      dst = lesser(*cur, end);
      src = bigger(*cur, end);
    }

    if (!dst || !src) break;
    deletes(dst, src, len);
    *cur = dst;
    if (*len) **len = '\0';
  } break;
  case 'C':
    handle_printable = insbind;
    [[fallthrough]];
  case 'D':
    *len = *cur;
    **len = '\0';
    break;
  case 'r':
    **cur = getchar();
    break;
  case '[':
    handleEs(getchar(), buf, cur, len);
    handle_printable = insbind;
    break;
  default:
    dispErr(__FUNCTION__, "unknown char: %c", c);
  }
}

/**
 * @brief Multi modal input function
 * @param[in] sz Buffer size
 * @param[out] buf Buffer
 * @return Is not CTRL-D pressed
 */
bool editline(int sz, char *buf) {
  bool ctrl_d;
  char *cur = buf;
  char *len = buf;

  struct termios orig_termios ondrop(disableRawMode);
  enableRawMode(&orig_termios);

  for (char c = getchar();; c = getchar()) {
    if ((ctrl_d = (c == ctrld)) || c == '\n' || buf + sz < len) break;

    switch (c) {
    case es:
      handle_printable = nrmbind;
      break;

    case backspace:
      if (cur <= buf) continue;
      memmove(cur - 1, cur, (size_t)(len - cur + 1));
      cur--;
      len--;
      break;

    default:
      handle_printable(c, buf, &cur, &len);
      break;
    }
    PRINT(ESEL(2) "\r", buf, "\033[", (int)(cur - buf + 1), "G");
  }
  putchar('\n');

  return !ctrl_d;
}
