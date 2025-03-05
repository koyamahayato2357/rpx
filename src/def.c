/**
 * @file src/def.c
 */

#include "def.h"
#include "testing.h"

test_table(
  bool_macro, , (int, int),
  {
    {0, BOOL(0)},
    {1, BOOL(1)},
    {1, BOOL(8)},
}
)
