#include "rc.h"
#include "main.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

void set_configpath(const char *specified_configpath, int len,
                    char *configpath) {
  const char *configdir = "/.config/rpx/";
  if (specified_configpath != nullptr)
    configdir = specified_configpath;

  const char *homedir = getenv("HOME") ?: ".";
  const int hdlen = strlen(homedir);
  const int cdlen = strlen(configdir);

  if (hdlen + cdlen + 1 >= len)
    return;

  memcpy(configpath, homedir, hdlen);
  memcpy(configpath + hdlen, configdir, cdlen + 1);
}

void load_initscript(const char *path) {
  char configpath[BUFSIZE];

  set_configpath(path, BUFSIZE, configpath);

  struct dirent *entry;
  DIR *dp = opendir(configpath);

  if (dp == nullptr)
    return;

  const int configpathlen = strlen(configpath);
  while ((entry = readdir(dp)) != nullptr) {
    if (entry->d_name[0] == '.') // ignore hidden files
      continue;
    char fname[BUFSIZE];
    strncpy(fname, configpath, configpathlen);
    strncpy(fname + configpathlen, entry->d_name, BUFSIZE - configpathlen);
    FILE *fp = fopen(fname, "r");
    reader_loop(fp);
    fclose(fp);
  }

  closedir(dp);
}
