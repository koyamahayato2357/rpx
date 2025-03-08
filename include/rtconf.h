/**
 * @file include/rtconf.h
 * @brief Define data types for runtime information
 */

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

  char prevexpr[buf_size];
  void (*plotexpr)(char const *);
} plotcfg_t /* plot config */;

typedef struct {
  real_t hist[buf_size];
  size_t histi;
  real_t reg[alpha_n];
} rrtinfo_t /* runtime info */;

typedef struct {
  elem_t hist[buf_size];
  size_t histi;
  elem_t reg[alpha_n];
} rtinfo_t /* runtime info */;

plotcfg_t getPlotCfg();
void setPlotCfg(plotcfg_t);
rtinfo_t getRuntimeInfo();
rrtinfo_t getRRuntimeInfo();
void setRuntimeInfo(rtinfo_t);
void setRRuntimeInfo(rrtinfo_t);
