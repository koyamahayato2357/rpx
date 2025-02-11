#pragma once
#include "main.h"
#include "sysconf.h"

#define ARGN 8

typedef struct {
  real_t stack[100];
  real_t *rbp;
  real_t *rsp;
  char const *expr;

  // to restore rsp
  // register the number of args used
  char max_argc[ARGN];
  _BitInt(3) unsigned max_argci;

  rrtinfo_t info;
  real_t *argv;
  bool iscontinue;

  // to restore argv
  // register the list of args
  real_t *callstack[8];
  _BitInt(3) unsigned callstacki;
} evalinfo_t;

elem_t eval_expr_real(char const *);
void rpx_eval(evalinfo_t *);
void init_evalinfo(evalinfo_t *);
