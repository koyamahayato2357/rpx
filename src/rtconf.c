/**
 * @file src/rtconf.c
 */

#include "rtconf.h"

plotcfg_t pcfg;
rrtinfo_t info_r = (rrtinfo_t){.histi = ~0UL};
rtinfo_t info_c = (rtinfo_t){.histi = ~0UL};

plotcfg_t getPlotCfg() {
  return pcfg;
}

void setPlotCfg(plotcfg_t pc) {
  pcfg = pc;
}

rtinfo_t getRuntimeInfo() {
  return info_c;
}
rrtinfo_t getRRuntimeInfo() {
  return info_r;
}

void setRuntimeInfo(rtinfo_t info) {
  info_c = info;
}
void setRRuntimeInfo(rrtinfo_t info) {
  info_r = info;
}
