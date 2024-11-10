#include "rc.h"
#include "chore.h"
#include "exproriented.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void set_configpath(char const *specified_configpath, int len,
                    char *configpath) {
  char const *configdir = specified_configpath ?: "/.config/rpx/";
  char const *homedir = getenv("HOME") ?: ".";
  int const cdlen = strlen(configdir);
  int const hdlen = strlen(homedir);

  if (hdlen + cdlen + 1 >= len)
    return;

  memcpy(configpath, homedir, hdlen);
  memcpy(configpath + hdlen, configdir, cdlen + 1);
}

void load_initscript(char const *path) {
  char configpath[BUFSIZE];

  set_configpath(path, BUFSIZE, configpath);

  struct dirent *entry;
  DIR *dp dropdir = opendir(configpath) ?: p$return();

  int const configpathlen = strlen(configpath);
  char fname[BUFSIZE];
  strncpy(fname, configpath, configpathlen);

  while ((entry = readdir(dp)) != nullptr) {
    if (entry->d_name[0] == '.') // ignore hidden files
      continue;
    strncpy(fname + configpathlen, entry->d_name, BUFSIZE - configpathlen);
    FILE *fp dropfile = fopen(fname, "r");
    reader_loop(fp);
  }
}
