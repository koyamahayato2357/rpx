#include "graphplot.h"
#include "evalfn.h"
#include "sysconf.h"
#include <sys/ioctl.h>

#define FONTROW 2
#define FONTCOL 1
#define FONTRATIO (double)((double)FONTCOL / FONTROW)

void init_plotconfig() {
  struct winsize w = get_winsz();
  int dispsz = lesser(w.ws_row, w.ws_col * FONTRATIO);

  plotcfg_t pcfg = get_plotcfg();
  pcfg.dispx = pcfg.dispy = dispsz - 5;
  pcfg.plotexpr = plotexpr;
  set_plotcfg(pcfg);
  set_pbounds(1, -1, 1, -1);
}

bool ispointgraph(double y0, double y1, double y) {
  plotcfg_t pcfg = get_plotcfg();

  return (y > y0 && y0 > y - pcfg.dy) || (y > y1 && y1 > y - pcfg.dy) ||
         (y0 > y && y > y1) || (y1 > y && y > y0) ||
         (y0 > y - pcfg.dy && y - pcfg.dy > y1) ||
         (y1 > y - pcfg.dy && y - pcfg.dy > y0);
}

static void drawaxisx(double const xn, int const dsplysz, double const dx) {
  putchar('\t');
  putchar('+');
  for (int i = 0; i < dsplysz / FONTRATIO; i++)
    putchar('-');
  putchar('\n');
  putchar('\t');
  printf("%.3lf", xn);
  for (int i = 0; i < dsplysz / FONTRATIO / 2; i++)
    putchar(' ');
  printf("%.3lf", xn + dx * dsplysz / 2 / FONTRATIO);
  putchar('\n');
}

void plotexpr(char const *expr) {
  plotcfg_t pcfg = get_plotcfg();
  evalinfo_t ei;

  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    real_t stack = (real_t){.elem = {.real = x0}, .isnum = true};
    init_evalinfo(&ei);
    ei.argv = &stack - 7;
    ei.expr = expr;
    rpx_eval(&ei);
    double y0 = ei.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / FONTRATIO; j++) {
      double x1 = pcfg.xn + pcfg.dx * j + pcfg.dx;
      stack = (real_t){.elem = {.real = x1}, .isnum = true};
      ei.expr = expr;
      rpx_eval(&ei);
      double y1 = ei.rsp->elem.real;
      putchar(ispointgraph(y0, y1, y) ? '*' : ' ');
      y0 = y1;
    }

    putchar('\n');
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

void plotexpr_implicit(char const *expr) {
  plotcfg_t pcfg = get_plotcfg();
  evalinfo_t ei;

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
    ei.argv = stack - 6;
    ei.expr = expr;
    rpx_eval(&ei);
    double res0 = ei.rsp->elem.real;
    for (int j = 0; j < pcfg.dispx / FONTRATIO; j++) {
      double x1 = pcfg.xn + pcfg.dx * (j + 1);
      stack[0] = (real_t){.elem = {.real = y1}, .isnum = true};
      stack[1] = (real_t){.elem = {.real = x1}, .isnum = true};
      ei.expr = expr;
      rpx_eval(&ei);
      double res1 = ei.rsp->elem.real;
      putchar(ispointgraph(res0, res1, 0) ? '*' : ' ');
      res0 = res1;
    }

    putchar('\n');
    y0 = y1;
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

void set_pbounds(double const xx, double const xn, double const yx,
                 double const yn) {
  plotcfg_t pcfg = get_plotcfg();

  pcfg.xx = xx;
  pcfg.xn = xn;
  pcfg.yx = yx;
  pcfg.yn = yn;

  pcfg.dx = (xx - xn) / pcfg.dispx * FONTRATIO;
  pcfg.dy = (yx - yn) / pcfg.dispy;

  set_plotcfg(pcfg);
}

void change_plotconfig(char const *cmd) {
  switch (*cmd++) {
  case 'd': { // display size
    plotcfg_t pcfg = get_plotcfg();

    int newx = eval_expr_real(cmd).elem.real;
    skip_untilcomma(&cmd);
    int newy = eval_expr_real(cmd).elem.real;

    int centerx = (pcfg.xx + pcfg.xn) / 2;
    int centery = (pcfg.yx + pcfg.yn) / 2;

    double coefx = (double)newx / pcfg.dispx;
    double coefy = (double)newy / pcfg.dispy;

    double diffxx = (pcfg.xx - centerx) * coefx;
    double diffxn = (pcfg.xn - centerx) * coefx;
    double diffyx = (pcfg.yx - centery) * coefy;
    double diffyn = (pcfg.yn - centery) * coefy;

    pcfg.xx = centerx + diffxx;
    pcfg.xn = centerx + diffxn;
    pcfg.yx = centery + diffyx;
    pcfg.yn = centery + diffyn;

    pcfg.dispx = newx;
    pcfg.dispy = newy;

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
  }
}
