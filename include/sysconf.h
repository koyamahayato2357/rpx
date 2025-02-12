#pragma once
#include "chore.h"
#include "main.h"

typedef struct {
  double xx; // max x
  double xn; // min x
  double yx; // max y
  double yn; // min y

  int dispx; // display x size
  int dispy; // display y size

  double dx; // increase in x per square
  double dy; // increase in y per square

  char prevexpr[BUFSIZE];
  void (*plotexpr)(char const *);
} plotcfg_t /* plot config */;

typedef struct {
  real_t hist[BUFSIZE];
  size_t histi;
  real_t reg[ALPN];
} rrtinfo_t /* runtime info */;

typedef struct {
  elem_t hist[BUFSIZE];
  size_t histi;
  elem_t reg[ALPN];
} rtinfo_t /* runtime info */;

plotcfg_t get_plotcfg();
void set_plotcfg(plotcfg_t);
rtinfo_t get_rtinfo();
rrtinfo_t get_rrtinfo();
void set_rtinfo(rtinfo_t);
void set_rrtinfo(rrtinfo_t);
