#pragma once
#ifndef NDEBUG
#include "gene.h"
#include <stdio.h>

typedef struct {
  bool enable;
  bool context;
  char *prefix;
  FILE *fp;
} ic_t;

extern ic_t ic_conf;

#define ic(...)                                                                \
  ({                                                                           \
    __VA_OPT__(auto x = __VA_ARGS__;)                                          \
    do {                                                                       \
      FILE *out_fp = ic_conf.fp ?: stderr;                                     \
      if (!ic_conf.enable)                                                     \
        break;                                                                 \
      fputs(ic_conf.prefix ?: "ic| ", out_fp);                                 \
      if (ic_conf.context)                                                     \
        fprintf(out_fp, HERE " in %s()" __VA_OPT__("- "), __FUNCTION__);       \
      __VA_OPT__(fputs(#__VA_ARGS__ ": ", out_fp); printany(x);)               \
      putchar('\n');                                                           \
    } while (0);                                                               \
    __VA_OPT__(x;)                                                             \
  })
#else
#define ic(...) __VA_ARGS__
#endif
