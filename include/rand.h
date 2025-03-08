/**
 * @file include/rand.h
 */

#pragma once
#include <stdint.h>

uint64_t xorsh();
double xorsh0to1();
void sxorsh(uint64_t);
