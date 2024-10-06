#pragma once

#ifndef TEST_MODE
void disperr(const char *, const char *, ...);
#else
#define disperr(a, b, ...)
#endif
