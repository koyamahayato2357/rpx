//! @file editline.c

#include "editline.h"
#include "chore.h"
#include "errcode.h"
#include "exception.h"
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
 * @throws ERR_CURSOR_OUT_OF_RANGE
 */
void movecur(int n, char *buf, char **cur, char *len) {
  if (n > len - *cur) {
    *cur = len;
    throw(ERR_CURSOR_OUT_OF_RANGE);
  } else if (-n > *cur - buf) {
    *cur = buf;
    throw(ERR_CURSOR_OUT_OF_RANGE);
  } else
    *cur += n;
}

test(movecur) {
  char buf[256] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  try movecur(1, buf, &cur, len);
  expect(cur == buf + 1);
  try movecur(3, buf, &cur, len);
  expect(cur == buf + 4);
  try movecur(-4, buf, &cur, len);
  expect(cur == buf);
  try movecur(-1, buf, &cur, len);
  expect(cur == buf);
  try movecur(999, buf, &cur, len);
  expect(cur == len);
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
  char *len = &str[strlen(str)];
  char *begin = str;
  char *end = &str[7];

  deletes(begin, end, &len);
  expect(!strcmp(str, "text."));
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
  char *newcur;

  if (dir > 0)
    newcur = findc(c, *cur, len);
  else
    newcur = findc_r(c, buf, *cur);

  *cur = newcur;
}

test(findmove) {
  char buf[256] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  findmove(' ', 1, buf, &cur, len);
  expect(*cur == ' ');
  findmove('e', 1, buf, &cur, len);
  expect(*cur == 'e');
  findmove('s', -1, buf, &cur, len);
  expect(*cur == 's');
  try {
    findmove('z', 1, buf, &cur, len);
    unreachable;
  }
  expect(*cur == 's');
  findmove('.', 1, buf, &cur, len);
  expect(*cur == '.');
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

  skipspcs(cur);
}

test(fwdw) {
  char buf[] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  fwdw(&cur, len);
  expect(*cur == 't');
  fwdw(&cur, len);
  expect(*cur == '.');
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
  expect(*cur == '.');
  bwdw(buf, &cur);
  expect(*cur == 't');
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
  skipspcs(cur);
}

test(fwdW) {
  char buf[] = "sample text.";
  char *cur = buf;
  char *len = buf + strlen(buf);

  fwdW(buf, &cur, len);
  expect(*cur == 't');
  fwdW(buf, &cur, len);
  expect(*cur == '\0');
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
  expect(*cur == 't');
  bwdW(buf, &cur, len);
  expect(*cur == 's');
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
  case '3':
    if (getchar() == '~') {
      if (*len == *cur)
        return;
      memmove(*cur, *cur + 1, *len - *cur - 1);
      (*len)--;
    }
    break;
  case 'C':
    try movecur(1, buf, cur, *len);
    break;
  case 'D':
    try movecur(-1, buf, cur, *len);
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
    try findmove(')', 1, buf, &cur, len);
    *end = cur;
    try findmove('(', -1, buf, &cur, len);
    *begin = cur + 1;
    break;
  case ']':
    try findmove(']', 1, buf, &cur, len);
    *end = cur;
    try findmove('[', -1, buf, &cur, len);
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
void inserts(int slen, const char *s, int curpos, char **cur, char **len,
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
  expect(!strcmp(buf, "sample textnew string."));
  expect(!strcmp(cur, "new string."));
  expect(len - buf == (long)strlen(buf));

  char str2[] = "extra string";
  inserts(strlen(str2), str2, 6, &cur, &len, buf + 256 - len);
  expect(!strcmp(buf, "sample textextra stringnew string."));
  expect(!strcmp(cur, "stringnew string."));
  expect(len - buf == (long)strlen(buf));
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

void (*handle_printable)(char c, char *buf, char **cur, char **len) = insbind;

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
    try movecur(-1, buf, cur, *len);
    break;
  case 'l':
    try movecur(1, buf, cur, *len);
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
    try findmove(getchar(), 1, buf, cur, *len);
    break;
  case 'F':
    try findmove(getchar(), -1, buf, cur, *len);
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
    if (input != 'i' && input != 'a') {
      try nrmbind(input, buf, &end, len);
      dst = lesser(*cur, end);
      src = bigger(*cur, end);
    } else
      try handle_txtobj(getchar(), buf, *cur, *len, &dst, &src);

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
    try handle_es(getchar(), buf, cur, len);
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

  struct termios orig_termios;
  enable_rawmode(&orig_termios);

  while ((c = getchar()) != '\n' && c != CTRL_D && len < sz + buf) {
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
      try handle_printable(c, buf, &cur, &len);
      break;
    }
    printf("\033[2K\r%s\r", buf);
    if (cur > buf)
      printf("\033[%ldC", cur - buf);
  }
  putchar('\n');
  disable_rawmode(&orig_termios);

  return c != CTRL_D;
}
