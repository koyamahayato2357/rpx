#include "errcode.h"

char const *codetomsg(int code) {
  switch (code) {
  case ERR_SUCCESS:
    return "success";
  case ERR_UNKNOWN_CHAR:
    return "Unknown character";
  case ERR_UNKNOWN_FN:
    return "Unknown function";
  case ERR_DIMENTION_MISMATCH:
    return "Dimention mismatch";
  case ERR_IRREGULAR_MATRIX:
    return "Irregular matrix";
  case ERR_NON_SQUARE_MATRIX:
    return "Non-square matrix";
  case ERR_TYPE_MISMATCH:
    return "Type mismatch";
  case ERR_FILE_NOT_FOUND:
    return "File not found";
  case ERR_BUFFER_DEPLETION:
    return "Buffer depletion";
  case ERR_CURSOR_OUT_OF_RANGE:
    return "Cursor out of range";
  case ERR_CHAR_NOT_FOUND:
    return "Character not found";
  case ERR_ALLOCATION_FAILURE:
    return "Allocation failure";
  case ERR_UNKNOWN_COMMAND:
    return "Unknown command";
    case ERR_UNKNOWN_OPTION:
      return "Unknown option";
  default:
    return "";
  }
}
