/**
 * @file src/graphplot.c
 * @brief Define graph related functions
 */

#include "graphplot.h"
#include "evalfn.h"
#include "rtconf.h"
#include "testing.h"
#include <sys/ioctl.h>

constexpr double fontrow = 2;
constexpr double fontcol = 1;
constexpr double font_ratio = fontcol / fontrow;

[[gnu::const]] static bool
isPointGraph(double y0, double y1, double y, double dy) {
  return (y > y0 && y0 > y - dy) || (y > y1 && y1 > y - dy)
      || (y0 > y && y > y1) || (y1 > y && y > y0)
      || (y0 > y - dy && y - dy > y1) || (y1 > y - dy && y - dy > y0);
}

test_table(
  ispoint, isPointGraph, (bool, double, double, double, double),
  {
    //        y0    y1    y   dy
    { true,  0.0,  1.0, 0.5, 1.0},
    { true,  0.0,  0.0, 0.5, 1.0},
    { true,  0.0, 10.0, 5.0, 0.5},
    { true,  1.0,  0.0, 0.5, 0.5},
    {false,  0.0,  0.0, 0.5, 0.5},
    {false, -1.0,  0.0, 0.5, 0.5},
    {false,  0.0,  0.0, 0.0, 0.5},
    {false,  1.0,  0.0, 5.0, 0.5},
}
)

static void drawAxisX(double const xn, int const disp_size, double const dx) {
  putchar('\t');
  putchar('+');
  for (int i = 0; i < disp_size / font_ratio; i++) putchar('-');
  putchar('\n');
  putchar('\t');
  printf("%.3lf", xn);
  for (int i = 0; i < disp_size / font_ratio / 2; i++) putchar(' ');
  printf("%.3lf", xn + dx * disp_size / 2 / font_ratio);
  putchar('\n');
}

[[gnu::nonnull]] void plotexpr(char const *restrict expr) {
  plotcfg_t pcfg = getPlotCfg();
  machine_t ei;

  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    real_t stack = (real_t){.elem = {.real = x0}, .isnum = true};
    initEvalinfo(&ei);
    ei.e.args = &stack - 7;
    ei.c.expr = expr;
    rpxEval(&ei);
    double y0 = ei.s.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / font_ratio; j++) {
      stack.elem.real = pcfg.xn + pcfg.dx * j + pcfg.dx;

      ei.c.expr = expr;
      rpxEval(&ei);
      double y1 = ei.s.rsp->elem.real;
      putchar(isPointGraph(y0, y1, y, pcfg.dy) ? '*' : ' ');
      fflush(stdout);
      y0 = y1;
    }

    putchar('\n');
  }

  drawAxisX(pcfg.xn, pcfg.dispx, pcfg.dx);
}

[[gnu::nonnull]] void plotexprImplicit(char const *restrict expr) {
  plotcfg_t pcfg = getPlotCfg();
  machine_t ei;

  double y0 = pcfg.yx + pcfg.dy;
  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    double y1 = pcfg.yx - pcfg.dy * (i - 1);
    real_t stack[2] = {
      (real_t){.elem = {.real = y0}, .isnum = true},
      (real_t){.elem = {.real = x0}, .isnum = true},
    };
    initEvalinfo(&ei);
    ei.e.args = stack - 6;
    ei.c.expr = expr;
    rpxEval(&ei);
    double res0 = ei.s.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / font_ratio; j++) {
      double x1 = pcfg.xn + pcfg.dx * (j + 1);
      stack[0] = (real_t){.elem = {.real = y1}, .isnum = true};
      stack[1] = (real_t){.elem = {.real = x1}, .isnum = true};
      ei.c.expr = expr;
      rpxEval(&ei);
      double res1 = ei.s.rsp->elem.real;
      putchar(isPointGraph(res0, res1, 0, pcfg.dy) ? '*' : ' ');
      res0 = res1;
    }

    putchar('\n');
    y0 = y1;
  }

  drawAxisX(pcfg.xn, pcfg.dispx, pcfg.dx);
}

static void setPlotBounds(
  double const xx, double const xn, double const yx, double const yn
) {
  plotcfg_t pcfg = getPlotCfg();

  pcfg.xx = xx;
  pcfg.xn = xn;
  pcfg.yx = yx;
  pcfg.yn = yn;

  pcfg.dx = (xx - xn) / pcfg.dispx * font_ratio;
  pcfg.dy = (yx - yn) / pcfg.dispy;

  setPlotCfg(pcfg);
}

void initPlotCfg() {
  struct winsize w = getWinSize();
  int dispsz = (int)lesser((double)w.ws_row, w.ws_col * font_ratio);

  plotcfg_t pcfg = getPlotCfg();
  pcfg.dispx = pcfg.dispy = dispsz - 5;
  pcfg.plotexpr = plotexpr;
  setPlotCfg(pcfg);
  setPlotBounds(1, -1, 1, -1);
}

[[gnu::nonnull]] void changePlotCfg(char const *cmd) {
  switch (*cmd++) {
  case 'd': { // display size
    plotcfg_t pcfg = getPlotCfg();

    double newx = evalExprReal(cmd).elem.real;
    skipUntilComma(&cmd);
    double newy = evalExprReal(cmd).elem.real;

    double centerx = (pcfg.xx + pcfg.xn) / 2;
    double centery = (pcfg.yx + pcfg.yn) / 2;

    double coefx = newx / pcfg.dispx;
    double coefy = newy / pcfg.dispy;

    double diffxx = (pcfg.xx - centerx) * coefx;
    double diffxn = (pcfg.xn - centerx) * coefx;
    double diffyx = (pcfg.yx - centery) * coefy;
    double diffyn = (pcfg.yn - centery) * coefy;

    pcfg.xx = centerx + diffxx;
    pcfg.xn = centerx + diffxn;
    pcfg.yx = centery + diffyx;
    pcfg.yn = centery + diffyn;

    pcfg.dispx = (int)newx;
    pcfg.dispy = (int)newy;

    setPlotCfg(pcfg);
  } break;
  case 'r': { // range
    double newxx = evalExprReal(cmd).elem.real;
    skipUntilComma(&cmd);
    double newxn = evalExprReal(cmd).elem.real;
    skipUntilComma(&cmd);
    double newyx = evalExprReal(cmd).elem.real;
    skipUntilComma(&cmd);
    double newyn = evalExprReal(cmd).elem.real;

    setPlotBounds(newxx, newxn, newyx, newyn);
  } break;
  default:
    [[clang::unlikely]];
  }
}
