#include "chore.h"
#include "exproriented.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

[[gnu::nonnull(3)]] void set_configpath(
  char const *specified_cfgpath, size_t len, char *restrict cfgpath
) {
  char const *configdir = specified_cfgpath ?: "/.config/rpx/";
  char const *homedir   = getenv("HOME") ?: ".";
  size_t const hdlen    = strlen(homedir);

  strncpy(cfgpath, homedir, len);
  strncpy(cfgpath + hdlen, configdir, len - hdlen);
}

void load_initscript(char const *path) {
  char configpath[BUFSIZE];

  set_configpath(path, BUFSIZE, configpath);

  DIR *dp dropdir = opendir(configpath) ?: p$return();

  size_t const configpathlen = strlen(configpath);
  char fname[BUFSIZE];
  strncpy(fname, configpath, configpathlen);

  for (struct dirent *entry; (entry = readdir(dp));) {
    if (entry->d_name[0] == '.') continue; // ignore hidden files
    strncpy(fname + configpathlen, entry->d_name, BUFSIZE - configpathlen);
    FILE *fp dropfile = fopen(fname, "r");
    reader_loop(fp);
  }
}
