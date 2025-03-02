/**
 * @file include/exproriented.h
 * @brief Define macros for expression oriented programming
 */

#pragma once
#include "errcode.h"

constexpr size_t UNIT = 0;

#define orelse ?:

#define $break         $(break)
#define $continue      $(continue)
#define $return(...)   $(return __VA_ARGS__)
#define $unreachable   $(unreachable)
#define $panic(e, ...) $(panic(e __VA_OPT__(, ) __VA_ARGS__))

// use in pointer calculation
#define p$break         p$(break)
#define p$continue      p$(continue)
#define p$return(a)     p$(return a)
#define p$unreachable   p$(unreachable)
#define p$panic(e, ...) p$(panic(e __VA_OPT__(, ) __VA_ARGS__))

#define $if(cond) (cond) ?
#define $else :
#define $(statements) \
  ({ \
    statements; \
    UNIT; \
  })
#define p$(statements) \
  ({ \
    statements; \
    (void *)UNIT; \
  })
