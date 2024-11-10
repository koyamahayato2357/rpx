#pragma once

#ifndef TEST_MODE
void disperr(char const *, char const *, ...);
#else
#define disperr(a, b, ...)
#endif
