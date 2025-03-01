#pragma once
#include "gene.h"
#include <stdio.h>

overloadable void disperr(FILE *, char const *, char const *, ...);
overloadable void disperr(char const *, char const *, ...);
