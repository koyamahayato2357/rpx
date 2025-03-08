/**
 * @file src/rc.c
 * @brief Define run commands processing functions
 */

#include "chore.h"
#include "exproriented.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

[[gnu::nonnull(3)]] static void
setCfgPath(char const *specified_cfgpath, size_t len, char *restrict cfgpath) {
  char const *configdir = specified_cfgpath ?: "/.config/rpx/";
  char const *homedir = getenv("HOME") ?: ".";
  size_t const hdlen = strlen(homedir);

  strncpy(cfgpath, homedir, len);
  strncpy(cfgpath + hdlen, configdir, len - hdlen);
}

void loadInitScript(char const *path) {
  char configpath[buf_size];

  setCfgPath(path, buf_size, configpath);

  size_t const configpathlen = strlen(configpath);
  char fname[buf_size];
  strncpy(fname, configpath, configpathlen);

  DIR *dp dropdir = opendir(configpath) ?: p$return();

  for (struct dirent const *entry; (entry = readdir(dp));) {
    if (entry->d_name[0] == '.') continue; // ignore hidden files
    strncpy(fname + configpathlen, entry->d_name, buf_size - configpathlen);
    FILE *fp dropfile = fopen(fname, "r");
    readerLoop(fp);
  }
}
