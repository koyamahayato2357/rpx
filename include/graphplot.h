#pragma once

void init_plotconfig();
bool ispointgraph(double, double, double);
void plotexpr(char const *);
void plotexpr_implicit(char const *);
void set_pbounds(double const, double const, double const, double const);
void change_plotconfig(char const *);
