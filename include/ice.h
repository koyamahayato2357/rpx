#pragma once
#ifdef ICECREAM
#include "gene.h"

typedef struct {
  bool enable;
  bool context;
  char *prefix;
  FILE *fp;
} ic_t;

extern ic_t ic_conf;

#define ic(...)                                                                \
  ({                                                                           \
    FILE *out_fp = ic_conf.fp ?: stderr;                                       \
    auto x = __VA_ARGS__;                                                      \
    if (!ic_conf.enable)                                                       \
      goto end;                                                                \
    fputs(ic_conf.prefix ?: "ic| ", out_fp);                                   \
    if (ic_conf.context) {                                                     \
      fputs(HERE " in ", out_fp);                                              \
      fputs(__FUNCTION__, out_fp);                                             \
      fputs("()- ", out_fp);                                                   \
    }                                                                          \
    __VA_OPT__(fputs(#__VA_ARGS__ ": ", out_fp); printany(x);)                 \
    putchar('\n');                                                             \
  end:                                                                         \
    x;                                                                         \
  })
#endif
