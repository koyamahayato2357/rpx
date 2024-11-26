#ifndef NDEBUG
#include "ice.h"
#include "testing.h"

ic_t ic_conf = {
  .enable = true,
  .context = false,
  .prefix = nullptr,
  .fp = nullptr,
};

#ifdef TEST_MODE_ALL
test(ice) {
  putchar('\n');
  ic(0);
  ic_conf.enable = false;
  ic();
  ic_conf.enable = true;
  ic(42);
  ic_conf.context = true;
  ic("hello world!");
  ic();
}
#endif
#endif
