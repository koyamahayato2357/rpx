#pragma once
#include "main.h"
#include "sysconf.h"

#define ARGN 8

typedef struct {
  double stack[100];
  double *rbp;
  double *rsp;
  char const *expr;

  // to restore rsp
  // register the number of args used
  char max_argc[ARGN];
  _BitInt(3) unsigned max_argci;

  rtinfo_t info;
  bool iscontinue;

  // to restore argv
  // register the list of args
  double *callstack[8];
  _BitInt(3) unsigned callstacki;
} evalinfo_t;

elem_t eval_expr_real(char const *);
elem_t eval_expr_real_with_info(evalinfo_t *);
