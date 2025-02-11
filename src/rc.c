#include "chore.h"
#include "exproriented.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void set_configpath(char const *_Nullable specified_cfgpath, int len,
                    char *_Nonnull restrict cfgpath) {
  char const *configdir = specified_cfgpath ?: "/.config/rpx/";
  char const *homedir = getenv("HOME") ?: ".";
  int const hdlen = strlen(homedir);

  strncpy(cfgpath, homedir, len);
  strncpy(cfgpath + hdlen, configdir, len - hdlen);
}

void load_initscript(char const *_Nullable path) {
  char configpath[BUFSIZE];

  set_configpath(path, BUFSIZE, configpath);

  DIR *dp dropdir = opendir(configpath) ?: p$return();

  int const configpathlen = strlen(configpath);
  char fname[BUFSIZE];
  strncpy(fname, configpath, configpathlen);

  for (struct dirent *entry; (entry = readdir(dp));) {
    if (entry->d_name[0] == '.')
      continue; // ignore hidden files
    strncpy(fname + configpathlen, entry->d_name, BUFSIZE - configpathlen);
    FILE *fp dropfile = fopen(fname, "r");
    reader_loop(fp);
  }
}
