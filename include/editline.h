#pragma once
#define ES '\033'
#define BS 127
#define CTRL_D 4

void movecur(int, char *, char **, char *);
void findmove(char, int, char *, char **, char *);
void inserts(int, char const *, int, char **, char **, int);
bool editline(int, char *);
