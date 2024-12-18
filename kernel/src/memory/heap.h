#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct HeapSeg
{
    size_t length;
    struct HeapSeg* next;
    struct HeapSeg* last;
    bool free;
} HeapSeg_t;

void InitializeHeap(void* heapAddress, size_t heapPages);

void* malloc(size_t size);
void free(void* address);


