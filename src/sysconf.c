#include "sysconf.h"

plotcfg_t pcfg;
rrtinfo_t info_r = (rrtinfo_t){.histi = ~(argc_t)0};
rtinfo_t info_c = (rtinfo_t){.histi = ~(argc_t)0};

plotcfg_t get_plotcfg() { return pcfg; }

void set_plotcfg(plotcfg_t pc) { pcfg = pc; }

rtinfo_t get_rtinfo() { return info_c; }
rrtinfo_t get_rrtinfo() { return info_r; }

void set_rtinfo(rtinfo_t info) { info_c = info; }
void set_rrtinfo(rrtinfo_t info) { info_r = info; }
