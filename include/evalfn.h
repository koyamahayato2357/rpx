/**
 * @file include/evalfn.h
 * @brief Define data types for evaluationg expressions in real number mode
 */

#pragma once
#include "main.h"
#include "rtconf.h"

constexpr size_t arg_n = 8;

typedef struct {
  real_t payload[buf_size];
  real_t *rbp, *rsp;
} stack_t;

typedef struct {
  rrtinfo_t info;
  real_t *args;
  bool iscontinue;
} env_t;

typedef struct {
  char const *expr;
  char const *rip;
} ctrl_t;

typedef struct {
  // to restore argv
  // register the list of args
  real_t *callstack[arg_n];
  unsigned callstacki;
  // to restore rsp
  // register the number of args used
  char argc[arg_n];
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

[[gnu::nonnull]] elem_t evalExprReal(char const *);
[[gnu::nonnull]] void rpxEval(machine_t *);
[[gnu::nonnull]] void initEvalinfo(machine_t *);
