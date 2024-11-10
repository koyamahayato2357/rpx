#pragma once

typedef enum {
  ERR_RETRY = 1,
  ERR_SUCCESS = 0,
  ERR_UNKNOWN_CHAR = -1,
  ERR_UNKNOWN_FN = -2,
  ERR_DIMENTION_MISMATCH = -3,
  ERR_IRREGULAR_MATRIX = -4,
  ERR_NON_SQUARE_MATRIX = -5,
  ERR_TYPE_MISMATCH = -6,
  ERR_FILE_NOT_FOUND = -7,
  ERR_BUFFER_DEPLETION = -8,
  ERR_CURSOR_OUT_OF_RANGE = -9,
  ERR_CHAR_NOT_FOUND = -10,
  ERR_ALLOCATION_FAILURE = -11,
  ERR_UNKNOWN_COMMAND = -12,
  ERR_REACHED_UNREACHABLE = -13,
  ERR_UNKNOWN_OPTION = -14
} errcode_t;

#define panic(e, ...)                                                          \
  do {                                                                         \
    printf("Panicked at " HERE " " __VA_ARGS__);                               \
    puts(codetomsg(e));                                                        \
    exit(e);                                                                   \
  } while (0)

char const *codetomsg(int);
