#include "sysconf.h"

plotcfg_t pcfg;
rtinfo_t info_r;
rtinfo_t info_c;

plotcfg_t get_plotcfg() { return pcfg; }

void set_plotcfg(plotcfg_t pc) { pcfg = pc; }

rtinfo_t get_rtinfo(char T) { return T == 'r' ? info_r : info_c; }

void set_rtinfo(char T, rtinfo_t info) {
  if (T == 'r')
    info_r = info;
  else
    info_c = info;
}
