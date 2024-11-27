/*
 * Usage
 * ----------------------------------------------------------------------------
 * Ignore error          | ignerr maybe_throw();
 * ----------------------------------------------------------------------------
 * Specific error        | try maybe_throw();
 *                       | catch (ERR_LIKE_THIS) puts("aaa");
 *                       | catch (ERR_LIKE_THAT) puts("bbb");
 *                       | catchor (ERR1 | ERR2) puts("ccc");
 * ----------------------------------------------------------------------------
 * All error             | try maybe_throw();
 *                       | catchany capture(errcode) err_handler(errcode);
 * ----------------------------------------------------------------------------
 * Propagate error       | maybe_throw();
 * ----------------------------------------------------------------------------
 * Multiple statements   | try {
 *                       |   maybe_throw();
 *                       |   probably_throw();
 *                       |   throw_by_any_chance();
 *                       | } catchany {
 *                       |   puts("Something went wrong");
 *                       |   exit(1);
 *                       | }
 * ----------------------------------------------------------------------------
 * Get error code        | try maybe_throw();
 *                       | catchany capture(errcode)
 *                       |   printf("This is error code: %d\n", errcode);
 * ----------------------------------------------------------------------------
 * Thrower               | throw(ERR_CODE_OF_TYPE_INT);
 * ----------------------------------------------------------------------------
 *
 * Reserved word
 * - `EXCEPTION_H_*`
 */

#include "exception.h"
#include "testing.h"
#ifdef TEST_MODE
#include "errcode.h"
#include <stdio.h>
#include <stdlib.h>
#endif

jmp_buf EXCEPTION_H_jb[MAX_NEST_DEPTH];
int EXCEPTION_H_errcode;
int EXCEPTION_H_nest;

void EXCEPTION_H_cl(int **g) { (**g)--; }

void _throw() { throw(-1); }
void _nothrow() {}

test(exception) {

  try {
    _throw();
    unreachable;
  }
  try _nothrow();
  catchany unreachable;
  try {
    try _throw();
    catchany capture(errcode) {
      if (errcode != -1)
        unreachable;
    }
  }
  try _throw();
  catchor(0) unreachable;
  // int i = 0;
  // try _throw();
  // catchany {
  //   if (++i < 5)
  //     retry;
  //   if (i < 5)
  //     unreachable;
  // }
}
