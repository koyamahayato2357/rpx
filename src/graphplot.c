#include "graphplot.h"
#include "chore.h"
#include "main.h"
#include "sysconf.h"
#include <stdio.h>
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

static void drawaxisx(const double xn, const int dsplysz, const double dx) {
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

void plotexpr(char *expr) {
  plotcfg_t pcfg = get_plotcfg();
  rtinfo_t info = get_rtinfo('r');

  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    info.usrfn.argv[0] = x0;
    set_rtinfo('r', info);
    double y0 = eval_expr_real(expr).elem.real;
    for (int j = 0; j < pcfg.dispx / FONTRATIO; j++) {
      double x1 = pcfg.xn + pcfg.dx * j + pcfg.dx;
      info.usrfn.argv[0] = x1;
      set_rtinfo('r', info);
      double y1 = eval_expr_real(expr).elem.real;
      if (ispointgraph(y0, y1, y))
        putchar('*');
      else
        putchar(' ');
      y0 = y1;
    }

    putchar('\n');
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

void plotexpr_implicit(char *expr) {
  plotcfg_t pcfg = get_plotcfg();
  rtinfo_t info = get_rtinfo('r');

  double y0 = pcfg.yn - pcfg.dy;
  for (int i = 0; i < pcfg.dispy; i++) {
    double y = pcfg.yx - pcfg.dy * i;
    printf("%.3lf\t|", y);
    double x0 = pcfg.xn - pcfg.dx;
    double y1 = pcfg.yx - pcfg.dy * i - pcfg.dy;
    info.usrfn.argv[0] = x0;
    info.usrfn.argv[1] = y0;
    set_rtinfo('r', info);
    double res0 = eval_expr_real(expr).elem.real;
    for (int j = 0; j < pcfg.dispx / FONTRATIO; j++) {
      double x1 = pcfg.xn + pcfg.dx * j + pcfg.dx;
      info.usrfn.argv[0] = x1;
      info.usrfn.argv[1] = y1;
      set_rtinfo('r', info);
      double res1 = eval_expr_real(expr).elem.real;
      if (ispointgraph(res0, res1, 0))
        putchar('*');
      else
        putchar(' ');
      res0 = res1;
    }

    putchar('\n');
    y0 = y1;
  }

  drawaxisx(pcfg.xn, pcfg.dispx, pcfg.dx);
}

void set_pbounds(const double xx, const double xn, const double yx,
                 const double yn) {
  plotcfg_t pcfg = get_plotcfg();

  pcfg.xx = xx;
  pcfg.xn = xn;
  pcfg.yx = yx;
  pcfg.yn = yn;

  pcfg.dx = (xx - xn) / pcfg.dispx * FONTRATIO;
  pcfg.dy = (yx - yn) / pcfg.dispy;

  set_plotcfg(pcfg);
}

void change_plotconfig(char *cmd) {
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
