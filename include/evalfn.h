#pragma once
#include "main.h"
#include "sysconf.h"

typedef struct {
  double stack[100];
  double *rbp;
  double *rsp;
  char const *expr;
  rtinfo_t info;
} evalinfo_t;

elem_t eval_expr_real(char const *);

