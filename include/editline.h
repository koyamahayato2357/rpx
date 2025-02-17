#pragma once
#include <stddef.h>

#define ES '\033'
#define BS 127
#define CTRL_D 4

void movecur(int, char *, char **, char *);
bool findmove(char, int, char *, char **);
void inserts(size_t, char const *, int, char **, char **, size_t);
bool editline(int, char *);
