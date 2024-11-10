#pragma once
#include "errcode.h"
#include "exception.h"

#define $break $(break)
#define $continue $(continue)
#define $return(...) $(return __VA_ARGS__)
#define $unreachable $(unreachable)
#define $panic(e, ...) $(panic(e __VA_OPT__(, ) __VA_ARGS__))
#define $throw(e) $(throw(e))

// use in pointer calculation
#define p$break p$(break)
#define p$continue p$(continue)
#define p$return(a) p$(return a)
#define p$unreachable p$(unreachable)
#define p$panic(e, ...) p$(panic(e __VA_OPT__(, ) __VA_ARGS__))
#define p$throw(e) p$(throw(e))

#define $if(cond) (cond) ?
#define $else :
#define $(statements)                                                          \
  ({                                                                           \
    statements;                                                                \
    0UL;                                                                       \
  })
#define p$(statements)                                                         \
  ({                                                                           \
    statements;                                                                \
    (void *)0;                                                                 \
  })
