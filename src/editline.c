//! @file editline.c

#include "editline.h"
#include "chore.h"
#include "errcode.h"
#include "exception.h"
#include "exproriented.h"
#include "main.h"
#include "testing.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

void enable_rawmode(struct termios *orig_termios) {
  struct termios raw;
  tcgetattr(STDIN_FILENO, orig_termios);
  raw = *orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_rawmode(struct termios *orig_termios) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

/**
 * @brief Move cursor
 * @param[in] n Number of moves
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in] len End of line
 */
void movecur(int n, char *buf, char **cur, char *len) {
  *cur = $if(n > len - *cur) len $else $if(-n > *cur - buf) buf $else * cur + n;
}

test(movecur) {
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
 * @param[in/out] len End of line
 */
void deletes(char *begin, char *end, char **len) {
  memcpy(begin, end, *len - end);
  *len -= end - begin;
  **len = '\0';
}

test(deletes) {
  char str[] = "sample text.";
  char *len = str + strlen(str);
  char *begin = str;
  char *end = str + 7;

  deletes(begin, end, &len);
  expecteq("text.", (char *)str);
}

/**
 * @brief Find character from a string
 * @param[in] c Character to look for
 * @param[in] cur Cursor pointer
 * @param[in] len End of line
 * @return Pointer to the location found
 * @throws ERR_CHAR_NOT_FOUND
 */
char *findc(char c, char *cur, char *len) {
  for (; cur <= len && *cur != c; cur++)
    ;

  if (cur > len)
    throw(ERR_CHAR_NOT_FOUND);

  return cur;
}

/**
 * @brief Find character from a string in opposite direction
 * @param[in] c Character to look for
 * @param[in] buf Start of line
 * @param[in] cur Cursor pointer
 * @return Pointer to the location found
 * @throws ERR_CHAR_NOT_FOUND
 */
char *findc_r(char c, char *buf, char *cur) {
  for (; cur >= buf && *cur != c; cur--)
    ;

  if (cur < buf)
    throw(ERR_CHAR_NOT_FOUND);

  return cur;
}

/**
 * @brief Find character and move there
 * @param[in] c Character to look for
 * @param[in] dir Search direction
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in] len End of line
 * @throws ERR_CHAR_NOT_FOUND
 */
void findmove(char c, int dir, char *buf, char **cur, char *len) {
  *cur = $if(dir > 0) findc(c, *cur, len) $else findc_r(c, buf, *cur);
}

test(findmove) {
  char buf[256] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  findmove(' ', 1, buf, &cur, len);
  expecteq(' ', *cur);
  findmove('e', 1, buf, &cur, len);
  expecteq('e', *cur);
  findmove('s', -1, buf, &cur, len);
  expecteq('s', *cur);
  ignerr {
    findmove('z', 1, buf, &cur, len);
    unreachable;
  }
  expecteq('s', *cur);
  findmove('.', 1, buf, &cur, len);
  expecteq('.', *cur);
}

/**
 * @brief Move the cursor forward one word
 * @param[in/out] cur Cursor pointer
 * @param[in] len End of line
 */
void fwdw(char **cur, char *len) {
  if (*cur >= len)
    return;

  int wasalnum = isalnum(**cur);
  int wasspace = isspace(**cur);
  (*cur)++;
  for (; !isspace(**cur) ^ wasspace && *cur != len &&
         !(isalnum(**cur) ^ wasalnum);
       (*cur)++)
    ;

  skipspcs((char const **)cur);
}

test(fwdw) {
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
 * @param[in/out] cur Cursor pointer
 */
void bwdw(char *buf, char **cur) {
  if (buf >= *cur)
    return;

  (*cur)--;
  for (; isspace(**cur); (*cur)--)
    ;

  int wasalnum = isalnum(**cur);
  for (; !isspace(*(*cur - 1)) && *cur != buf &&
         !(isalnum(*(*cur - 1)) ^ wasalnum);
       (*cur)--) {
  }
}

test(bwdw) {
  char buf[] = "sample text.";
  char *cur = buf + strlen(buf);

  bwdw(buf, &cur);
  expecteq(buf + 11, cur);
  bwdw(buf, &cur);
  expecteq(buf + 7, cur);
}

/**
 * @brief Move the cursor forward one WORD
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in] len End of line
 */
void fwdW(char *buf, char **cur, char *len) {
  try findmove(' ', 1, buf, cur, len);
  catchany *cur = len;
  skipspcs((char const **)cur);
}

test(fwdW) {
  char buf[] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  fwdW(buf, &cur, len);
  expecteq('t', *cur);
  fwdW(buf, &cur, len);
  expecteq('\0', *cur);
}

/**
 * @brief Move the cursor backward one WORD
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in] len End of line
 */
void bwdW(char *buf, char **cur, char *len) {
  for ((*cur)--; isspace(**cur); (*cur)--)
    ;
  try {
    findmove(' ', -1, buf, cur, len);
    (*cur)++;
  }
  catchany *cur = buf;
}

test(bwdW) {
  char buf[] = "sample text.";
  char *cur = buf + strlen(buf);
  char *len = buf + strlen(buf);

  bwdW(buf, &cur, len);
  expecteq('t', *cur);
  bwdW(buf, &cur, len);
  expecteq('s', *cur);
}

/**
 * @brief Handle escape sequence
 * @param[in] key Escape sequence
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in/out] len End of line
 * @throws ERR_UNKNOWN_CHAR
 */
void handle_es(char key, char *buf, char **cur, char **len) {
  switch (key) {
  case '3': // delete key
    if (getchar() != '~')
      break;
    if (*len == *cur)
      break;
    memmove(*cur, *cur + 1, *len - *cur - 1);
    (*len)--;
    break;
  case 'C':
    movecur(1, buf, cur, *len);
    break;
  case 'D':
    movecur(-1, buf, cur, *len);
    break;
  case 'F':
    *cur = *len;
    break;
  case 'H':
    *cur = buf;
  default:
    throw(ERR_UNKNOWN_CHAR);
  }
}

/**
 * @brief Handle text object
 * @param[in] textobj Second character of text object
 * @param[in] buf Start of line
 * @param[in] cur Cursor pointer
 * @param[in] len End of line
 * @param[out] begin Beginning of selected range of text object
 * @param[out] end End of selected range of text object
 * @throws ERR_UNKNOWN_CHAR
 */
void handle_txtobj(char txtobj, char *buf, char *cur, char *len, char **begin,
                   char **end) {
  switch (txtobj) {
  case 'w':
    fwdw(&cur, len);
    *end = cur;
    bwdw(buf, &cur);
    *begin = cur;
    break;
  case 'W':
    fwdW(buf, &cur, len);
    *end = cur;
    bwdW(buf, &cur, len);
    *begin = cur;
    break;
  case 'b':
    ignerr findmove(')', 1, buf, &cur, len);
    *end = cur;
    ignerr findmove('(', -1, buf, &cur, len);
    *begin = cur + 1;
    break;
  case ']':
    ignerr findmove(']', 1, buf, &cur, len);
    *end = cur;
    ignerr findmove('[', -1, buf, &cur, len);
    *begin = cur + 1;
    break;
  default:
    throw(ERR_UNKNOWN_CHAR);
    break;
  }
}

/**
 * @brief Insert a character at cursor position
 * @param[in] c Character to be inserted
 * @param[in/out] cur Cursor pointer
 * @param[in/out] len End of line
 * @detail Possible buffer overrun
 */
void insertc(char c, char **cur, char **len) {
  if (*cur != *len)
    memmove(*cur + 1, *cur, *len - *cur);
  *(*cur)++ = c;
  *++*len = '\0';
}

/**
 * @brief Insert a string at cursor position
 * @param[in] slen Number of characters in string to be inserted
 * @param[in] s String to be inserted
 * @param[in] curpos New cursor position
 * @param[in/out] cur Cursor pointer
 * @param[in/out] len End of line
 * @param[in] margin Number of characters can be added to the buffer
 * @throws ERR_BUFFER_DEPLETION
 */
void inserts(int slen, char const *s, int curpos, char **cur, char **len,
             int margin) {
  if (slen > margin)
    throw(ERR_BUFFER_DEPLETION);
  if (*cur != *len)
    memmove(*cur + slen, *cur, *len - *cur);
  memcpy(*cur, s, slen);
  *cur += curpos;
  *len += slen;
  **len = '\0';
}

test(inserts) {
  char buf[256] = "sample text.";
  char *len = buf + strlen(buf);
  char *cur = len - 1;

  char str1[] = "new string";
  inserts(strlen(str1), str1, 0, &cur, &len, buf + 256 - len);
  expecteq("sample textnew string.", (char *)buf);
  expecteq("new string.", cur);
  expecteq(strlen(buf), (size_t)(len - buf));

  char str2[] = "extra string";
  inserts(strlen(str2), str2, 6, &cur, &len, buf + 256 - len);
  expecteq("sample textextra stringnew string.", (char *)buf);
  expecteq("stringnew string.", cur);
  expecteq(strlen(buf), (size_t)(len - buf));
}

/**
 * @brief Behavior in insert mode
 * @param[in] c Typed character
 * @param[in] buf Start of line
 * @param[in/out] cur Cursor pointer
 * @param[in/out] len End of line
 * @throws ERR_BUFFER_DEPLETION
 */
void insbind(char c, char *buf, char **cur, char **len) {
  switch (c) {
  case '(':
    inserts(2, "()", 1, cur, len, buf + BUFSIZE - *len);
    break;

  case ')': {
    char *newcur;
    try newcur = findc(')', *cur, *len);
    catch (ERR_CHAR_NOT_FOUND) goto dflt;
    *cur = newcur + 1;
  } break;

  case '[':
    inserts(2, "[]", 1, cur, len, buf + BUFSIZE - *len);
    break;

  case ']': {
    char *newcur;
    try newcur = findc(']', *cur, *len);
    catch (ERR_CHAR_NOT_FOUND) goto dflt;
    *cur = newcur + 1;
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
 * @param[in/out] cur Cursor pointer
 * @param[in/out] len End of line
 * @throws ERR_CHAR_NOT_FOUND ERR_UNKNOWN_CHAR
 */
void nrmbind(char c, char *buf, char **cur, char **len) {
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
    fwdW(buf, cur, *len);
    break;
  case 'B':
    bwdW(buf, cur, *len);
    break;
  case 'f':
    ignerr findmove(getchar(), 1, buf, cur, *len);
    break;
  case 'F':
    ignerr findmove(getchar(), -1, buf, cur, *len);
    break;
  case 't':
    try findmove(getchar(), 1, buf, cur, *len);
    catch (ERR_CHAR_NOT_FOUND) break;
    (*cur)--;
    break;
  case 'T':
    try findmove(getchar(), -1, buf, cur, *len);
    catch (ERR_CHAR_NOT_FOUND) break;
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
      ignerr handle_txtobj(getchar(), buf, *cur, *len, &dst, &src);
    } else {
      ignerr nrmbind(input, buf, &end, len);
      dst = lesser(*cur, end);
      src = bigger(*cur, end);
    }

    deletes(dst, src, len);
    *cur = dst;
    **len = '\0';
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
    ignerr handle_es(getchar(), buf, cur, len);
    handle_printable = insbind;
    break;
  default:
    throw(ERR_UNKNOWN_CHAR);
  }
}

/**
 * @brief Multi modal input function
 * @param[in] sz Buffer size
 * @param[out] buf Buffer
 * @return Is not CTRL-D pressed
 */
bool editline(int sz, char *buf) {
  int c = 0;
  char *cur = buf;
  char *len = buf;

  struct termios orig_termios __attribute__((cleanup(disable_rawmode)));
  // getchar() becomes like getch() in MSVC
  enable_rawmode(&orig_termios);

  while (true) {
    c = getchar();
    if (c == '\n' || c == CTRL_D || buf + sz < len)
      break;

    switch (c) {
    case ES:
      handle_printable = nrmbind;
      break;

    case BS:
      if (cur <= buf)
        continue;
      memmove(cur - 1, cur, len - cur + 1);
      cur--;
      len--;
      break;

    default:
      ignerr handle_printable(c, buf, &cur, &len);
      break;
    }
    printf(ESEL(2) "\r%s\033[%ldG", buf, cur - buf + 1);
  }
  putchar('\n');

  return c != CTRL_D;
}
