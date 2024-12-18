#include "heap.h"
#include <paging.h>
#include <pmm.h>

void* heapStart;
void* heapEnd;
HeapSeg_t* lastSeg;

void heap_SplitSeg(HeapSeg_t* seg, size_t splitSize);
void heap_ExpandHeap(size_t size);

void heap_CombindForward(HeapSeg_t* seg);
void heap_CombineBackward(HeapSeg_t* seg);

void InitializeHeap(void* heapAddress, size_t heapPages)
{
    void* pos = heapAddress;

    for (size_t i = 0; i < heapPages; i++)
    {
        paging_MapMemory(pos, pmm_AllocatePage());
        pos = (void*)((size_t)pos + 0x1000);
    }

    size_t heapLength = heapPages * 0x1000;

    heapStart = heapAddress;
    heapEnd = (void*)((size_t)heapStart + heapLength);
    HeapSeg_t* startSeg = (HeapSeg_t*)heapStart;
    startSeg->length = heapLength - sizeof(HeapSeg_t);
    startSeg->next = NULL;
    startSeg->last = NULL;
    startSeg->free = true;

    lastSeg = startSeg;
}

void* malloc(size_t size)
{
    // Round it up to a multiple of 0x10
    if (size % 0x10 > 0)
    {
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) return NULL;

    HeapSeg_t* currentSeg = (HeapSeg_t*)heapStart;
    while (true)
    {
        if (currentSeg->free)
        {
            if (currentSeg->length > size)
            {
                heap_SplitSeg(currentSeg, size);
                currentSeg->free = false;
                return (void*)((uint64_t)currentSeg + sizeof(HeapSeg_t));
            }
            if (currentSeg->length == size)
            {
                // No need to split
                currentSeg->free = false;
                return (void*)((uint64_t)currentSeg + sizeof(HeapSeg_t));
            }
        }

        if (currentSeg->next == NULL) break;
        currentSeg = currentSeg->next;
    }

    heap_ExpandHeap(size);
    return malloc(size);
}

void free(void* address)
{
    HeapSeg_t* seg = (HeapSeg_t*)address - 1;
    seg->free = true;
    heap_CombindForward(seg);
    heap_CombineBackward(seg);
}


void heap_SplitSeg(HeapSeg_t* seg, size_t splitSize)
{
    HeapSeg_t* newSeg = (HeapSeg_t*)((void*)seg + splitSize + sizeof(HeapSeg_t));

    newSeg->length = seg->length - splitSize - sizeof(HeapSeg_t);
    newSeg->free = true;
    newSeg->next = seg->next;

    seg->length = splitSize;
    seg->free = false;
    seg->next = newSeg;

    if (lastSeg == seg) lastSeg = newSeg;
}

void heap_ExpandHeap(size_t size)
{
    // Round it up to a multiple of 0x1000 (page size)
    if (size % 0x1000 > 0)
    {
        size -= (size % 0x1000);
        size += 0x1000;
    }

    size_t pagesCount = size / 0x1000;
    HeapSeg_t* newSeg = (HeapSeg_t*)heapEnd;

    for (size_t i = 0; i < pagesCount; i++)
    {
        paging_MapMemory(heapEnd, pmm_AllocatePage());
        heapEnd = (void*)((size_t)heapEnd + 0x1000);
    }

    newSeg->free = true;
    newSeg->last = lastSeg;
    lastSeg->next = newSeg;
    newSeg->next = NULL;
    newSeg->length = size - sizeof(HeapSeg_t);
    heap_CombineBackward(newSeg);
}


void heap_CombindForward(HeapSeg_t* seg)
{
    if (seg->next == NULL) return;
    if (!seg->free) return;

    if (seg->next == lastSeg) lastSeg = seg;

    if (seg->next->next != NULL)
    {
        seg->next->next->last = seg;
    }

    seg->length = seg->length + seg->next->length + sizeof(HeapSeg_t);

    seg->next = seg->next->next;
}

void heap_CombineBackward(HeapSeg_t* seg)
{
    if (seg->last != NULL && seg->last->free)
    {
        heap_CombindForward(seg->last);
    }
}