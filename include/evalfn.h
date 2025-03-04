/**
 * @file include/evalfn.h
 * @brief Define data types for evaluationg expressions in real number mode
 */

#pragma once
#include "main.h"
#include "rtconf.h"

constexpr size_t ARGN = 8;

typedef struct {
  real_t payload[BUFSIZE];
  real_t *rbp, *rsp;
} stack_t;

typedef struct {
  rrtinfo_t info;
  real_t *args;
  bool iscontinue;
} env_t;

typedef struct {
  char const *expr;
} ctrl_t;

typedef struct {
  // to restore argv
  // register the list of args
  real_t *callstack[8];
  unsigned callstacki;
  // to restore rsp
  // register the number of args used
  char argc[ARGN];
  unsigned argci;
} dump_t;

/**
 * @brief Based on SECD machine
 */
typedef struct {
  stack_t s;
  env_t e;
  ctrl_t c;
  dump_t d;
} machine_t;

[[gnu::nonnull]] elem_t eval_expr_real(char const *);
[[gnu::nonnull]] void rpx_eval(machine_t *);
[[gnu::nonnull]] void init_evalinfo(machine_t *);
