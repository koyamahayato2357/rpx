#pragma once
#include "errcode.h"

#define $break $(break)
#define $continue $(continue)
#define $return(a) $(return a)
#define $unreachable $(unreachable)
#define $panic(e) $(panic(e))
#define $throw(e) $(throw(e))

// use in pointer calculation
#define p$break p$(break)
#define p$continue p$(continue)
#define p$return(a) p$(return a)
#define p$unreachable p$(unreachable)
#define p$panic(e) p$(panic(e))
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
