#pragma once
#include "main.h"
#include "sysconf.h"

#define ARGN 8

typedef struct {
  elem_t stack[100];
  elem_t *rbp;
  elem_t *rsp;
  char const *expr;

  // to restore rsp
  // register the number of args used
  char max_argc[ARGN];
  _BitInt(3) unsigned max_argci;

  rtinfo_t info;
  elem_t *argv;
  bool iscontinue;

  // to restore argv
  // register the list of args
  elem_t *callstack[8];
  _BitInt(3) unsigned callstacki;
} evalinfo_t;

elem_t eval_expr_real(char const *);
elem_t eval_expr_real_with_info(evalinfo_t *);
