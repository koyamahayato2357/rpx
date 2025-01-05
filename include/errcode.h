#pragma once
#include <stdlib.h>

typedef enum {
  ERR_SUCCESS,
  ERR_UNKNOWN_CHAR,
  ERR_UNKNOWN_FN,
  ERR_DIMENTION_MISMATCH,
  ERR_IRREGULAR_MATRIX,
  ERR_NON_SQUARE_MATRIX,
  ERR_TYPE_MISMATCH,
  ERR_FILE_NOT_FOUND,
  ERR_BUFFER_DEPLETION,
  ERR_CURSOR_OUT_OF_RANGE,
  ERR_CHAR_NOT_FOUND,
  ERR_ALLOCATION_FAILURE,
  ERR_UNKNOWN_COMMAND,
  ERR_REACHED_UNREACHABLE,
  ERR_UNKNOWN_OPTION,
} errcode_t;

#define panic(e, ...)                                                          \
  do {                                                                         \
    printf("Panicked at " HERE " " __VA_ARGS__);                               \
    puts(codetomsg(e));                                                        \
    abort();                                                                   \
  } while (0)

char const *codetomsg(int);
