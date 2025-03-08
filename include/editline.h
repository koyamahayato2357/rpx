/**
 * @file include/editline.h
 */

#pragma once
#include <stddef.h>

constexpr char es = '\033';
constexpr char backspace = 127;
constexpr char ctrld = 4;

bool editline(int, char *);
