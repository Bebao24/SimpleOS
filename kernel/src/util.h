#pragma once
#include <stdbool.h>

#define EXPORT_BYTE(target, first)                                             \
  ((first) ? ((target) & ~0xFF00) : (((target) & ~0x00FF) >> 8))
