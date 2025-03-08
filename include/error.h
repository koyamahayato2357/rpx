/**
 * @file include/error.h
 */

#pragma once
#include "gene.h"
#include <stdio.h>

[[gnu::format(printf, 3, 4)]] overloadable void
dispErr(FILE *, char const *, char const *, ...);

[[gnu::format(printf, 2, 3)]] overloadable void
dispErr(char const *, char const *, ...);
