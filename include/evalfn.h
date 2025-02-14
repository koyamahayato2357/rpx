#pragma once
#include "main.h"
#include "sysconf.h"

#define ARGN 8

typedef struct {
  real_t payload[BUFSIZE];
  real_t *rbp, *rsp;
} stack_t;

typedef struct {
  rrtinfo_t info;
  real_t *argv;
  bool iscontinue;
} env_t;

typedef struct {
  char const *expr;
} ctrl_t;

typedef struct {
  // to restore argv
  // register the list of args
  real_t *callstack[8];
  _BitInt(3) unsigned callstacki;
  // to restore rsp
  // register the number of args used
  char argc[ARGN];
  _BitInt(3) unsigned argci;
} dump_t;

typedef struct {
  stack_t s;
  env_t e;
  ctrl_t c;
  dump_t d;
} machine_t;

elem_t eval_expr_real(char const *);
void rpx_eval(machine_t *);
void init_evalinfo(machine_t *);
