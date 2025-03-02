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
constexpr double fontratio = fontcol / fontrow;

void init_plotconfig() {
  struct winsize w = get_winsz();
  int dispsz = (int)lesser((double)w.ws_row, w.ws_col * fontratio);

  plotcfg_t pcfg = get_plotcfg();
  pcfg.dispx = pcfg.dispy = dispsz - 5;
  pcfg.plotexpr = plotexpr;
  set_plotcfg(pcfg);
  set_pbounds(1, -1, 1, -1);
}

bool ispointgraph(double y0, double y1, double y, double dy) {
  return (y > y0 && y0 > y - dy) || (y > y1 && y1 > y - dy)
      || (y0 > y && y > y1) || (y1 > y && y > y0)
      || (y0 > y - dy && y - dy > y1) || (y1 > y - dy && y - dy > y0);
}

test_table(
  ispoint, ispointgraph, (bool, double, double, double, double),
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

static void drawaxisx(double const xn, int const dsplysz, double const dx) {
  putchar('\t');
  putchar('+');
  for (int i = 0; i < dsplysz / fontratio; i++) putchar('-');
  putchar('\n');
  putchar('\t');
  printf("%.3lf", xn);
  for (int i = 0; i < dsplysz / fontratio / 2; i++) putchar(' ');
  printf("%.3lf", xn + dx * dsplysz / 2 / fontratio);
  putchar('\n');
}

[[gnu::nonnull]] void plotexpr(char const *restrict expr) {
  plotcfg_t pcfg = get_plotcfg();
  machine_t ei;

  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    real_t stack = (real_t){.elem = {.real = x0}, .isnum = true};
    init_evalinfo(&ei);
    ei.e.args = &stack - 7;
    ei.c.expr = expr;
    rpx_eval(&ei);
    double y0 = ei.s.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / fontratio; j++) {
      stack.elem.real = pcfg.xn + pcfg.dx * j + pcfg.dx;

      ei.c.expr = expr;
      rpx_eval(&ei);
      double y1 = ei.s.rsp->elem.real;
      putchar(ispointgraph(y0, y1, y, pcfg.dy) ? '*' : ' ');
      fflush(stdout);
      y0 = y1;
    }

    putchar('\n');
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

[[gnu::nonnull]] void plotexpr_implicit(char const *restrict expr) {
  plotcfg_t pcfg = get_plotcfg();
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
    init_evalinfo(&ei);
    ei.e.args = stack - 6;
    ei.c.expr = expr;
    rpx_eval(&ei);
    double res0 = ei.s.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / fontratio; j++) {
      double x1 = pcfg.xn + pcfg.dx * (j + 1);
      stack[0] = (real_t){.elem = {.real = y1}, .isnum = true};
      stack[1] = (real_t){.elem = {.real = x1}, .isnum = true};
      ei.c.expr = expr;
      rpx_eval(&ei);
      double res1 = ei.s.rsp->elem.real;
      putchar(ispointgraph(res0, res1, 0, pcfg.dy) ? '*' : ' ');
      res0 = res1;
    }

    putchar('\n');
    y0 = y1;
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

void set_pbounds(
  double const xx, double const xn, double const yx, double const yn
) {
  plotcfg_t pcfg = get_plotcfg();

  pcfg.xx = xx;
  pcfg.xn = xn;
  pcfg.yx = yx;
  pcfg.yn = yn;

  pcfg.dx = (xx - xn) / pcfg.dispx * fontratio;
  pcfg.dy = (yx - yn) / pcfg.dispy;

  set_plotcfg(pcfg);
}

[[gnu::nonnull]] void change_plotconfig(char const *cmd) {
  switch (*cmd++) {
  case 'd': { // display size
    plotcfg_t pcfg = get_plotcfg();

    double newx = eval_expr_real(cmd).elem.real;
    skip_untilcomma(&cmd);
    double newy = eval_expr_real(cmd).elem.real;

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

    set_plotcfg(pcfg);
  } break;
  case 'r': { // range
    double newxx = eval_expr_real(cmd).elem.real;
    skip_untilcomma(&cmd);
    double newxn = eval_expr_real(cmd).elem.real;
    skip_untilcomma(&cmd);
    double newyx = eval_expr_real(cmd).elem.real;
    skip_untilcomma(&cmd);
    double newyn = eval_expr_real(cmd).elem.real;

    set_pbounds(newxx, newxn, newyx, newyn);
  } break;
  default:
    [[clang::unlikely]];
  }
}
