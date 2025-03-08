/**
 * @file include/editline.h
 */

#pragma once
#include <stddef.h>

constexpr char ES = '\033';
constexpr char BS = 127;
constexpr char CTRL_D = 4;

bool editline(int, char *);
