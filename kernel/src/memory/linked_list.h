#pragma once
#include <stddef.h>

typedef struct LLHeader LLHeader;

struct LLHeader
{
    LLHeader* next;
};

void* LinkedListAllocate(void** LLfirstPtr, size_t structSize);
