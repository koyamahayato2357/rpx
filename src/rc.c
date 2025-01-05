#include "chore.h"
#include "exproriented.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void set_configpath(char const *specified_cfgpath, int len, char *cfgpath) {
  char const *configdir = specified_cfgpath ?: "/.config/rpx/";
  char const *homedir = getenv("HOME") ?: ".";
  int const hdlen = strlen(homedir);

  strncpy(cfgpath, homedir, len);
  strncpy(cfgpath + hdlen, configdir, len - hdlen);
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
    if (entry->d_name[0] == '.')
      continue; // ignore hidden files
    strncpy(fname + configpathlen, entry->d_name, BUFSIZE - configpathlen);
    FILE *fp dropfile = fopen(fname, "r");
    reader_loop(fp);
  }
}
