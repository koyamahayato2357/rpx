#pragma once
#include <stddef.h>

constexpr char ES = '\033';
constexpr char BS = 127;
constexpr char CTRL_D = 4;

void movecur(int, char *, char **, char *);
bool findmove(char, int, char *, char **);
void inserts(size_t, char const *, int, char **, char **, size_t);
bool editline(int, char *);
