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
  void (*plotexpr)(char *);
} plotcfg_t;

typedef struct {
  char expr[ALPN][BUFSIZE];
  size_t argc[ALPN];
  double argv[9];
} uf_t;

typedef struct {
  elem_t hist[BUFSIZE];
  size_t histi;
  elem_t usrvar[ALPN];
  uf_t usrfn;
} rtinfo_t;

plotcfg_t get_plotcfg();
void set_plotcfg(plotcfg_t);
rtinfo_t get_rtinfo(char);
void set_rtinfo(char, rtinfo_t);
