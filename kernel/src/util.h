#pragma once
#include <stdbool.h>
#include <stdint.h>

#define EXPORT_BYTE(target, first)                                             \
  ((first) ? ((target) & ~0xFF00) : (((target) & ~0x00FF) >> 8))

#define COMBINE_WORD(firstWord, secondWord) (((uint32_t)(firstWord) << 16) | (secondWord))
