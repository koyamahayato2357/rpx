#pragma once
#include "main.h"
#include "sysconf.h"

typedef struct {
  double stack[100];
  double *rbp;
  double *rsp;
  char const *expr;
  char max_argc[8];
  _BitInt(3) max_argci;
  rtinfo_t info;
} evalinfo_t;

elem_t eval_expr_real(char const *);

