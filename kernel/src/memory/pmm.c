#include "pmm.h"
#include <stdbool.h>
#include <stddef.h>
#include <memory.h>
#include <bitmap.h>

uint64_t freeMemory;
uint64_t usedMemory;
uint64_t reservedMemory;
static bool Initialize = false;

static Bitmap g_Bitmap;

void pmm_InitializeBitmap(size_t bitmapSize, void* bufferAddr);

void InitializePMM(EFI_MEMORY_DESCRIPTOR* mMap, uint64_t mMapSize, uint64_t mDescriptorSize)
{
    if (Initialize) return;

    Initialize = true;

    uint64_t mMapEntries = mMapSize / mDescriptorSize;

    void* largestFreeSeg = NULL;
    size_t largestFreeSegSize = 0;

    for (uint64_t i = 0; i < mMapEntries; i++)
    {
        EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mDescriptorSize));
        if (descriptor->type == 7)
        {
            // free memory
            if (descriptor->numPages * 0x1000 > largestFreeSegSize)
            {
                largestFreeSeg = descriptor->physicalAddr;
                largestFreeSegSize = descriptor->numPages * 0x1000;
            }
        }
    }

    uint64_t memorySize = GetMemorySize(mMap, mMapEntries, mDescriptorSize);
    freeMemory = memorySize;
    uint64_t bitmapSize = memorySize / 0x1000 / 8 + 1;

    pmm_InitializeBitmap(bitmapSize, largestFreeSeg);

    pmm_ReservePages(0, memorySize / 0x1000 + 1);
    for (uint64_t i = 0; i < mMapEntries; i++)
    {
        EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mDescriptorSize));
        if (descriptor->type == 7)
        {
            // free memory
            pmm_UnreservePages(descriptor->physicalAddr, descriptor->numPages);
        }
    }

    pmm_ReservePages(0, 0x10);
    pmm_LockPages(g_Bitmap.bitmapBuffer, g_Bitmap.bitmapSize / 0x1000 + 1);
}

void pmm_InitializeBitmap(size_t bitmapSize, void* bufferAddr)
{
    g_Bitmap.bitmapSize = bitmapSize;
    g_Bitmap.bitmapBuffer = (uint8_t*)bufferAddr;

    memset(g_Bitmap.bitmapBuffer, 0, bitmapSize);
}

void* pmm_AllocatePage()
{
    for (uint64_t i = 0; i < g_Bitmap.bitmapSize * 8; i++)
    {
        if (Bitmap_Get(&g_Bitmap, i)) continue;
        pmm_LockPage((void*)(i * 0x1000));
        return (void*)(i * 0x1000);
    }

    return NULL;
}

void pmm_FreePage(void* address)
{
    uint64_t index = (uint64_t)address / 4096;
    if (Bitmap_Get(&g_Bitmap, index) == false) return;
    Bitmap_Set(&g_Bitmap, index, false);
    freeMemory += 0x1000;
    usedMemory -= 0x1000;
}

void pmm_LockPage(void* address)
{
    uint64_t index = (uint64_t)address / 4096;
    if (Bitmap_Get(&g_Bitmap, index) == true) return;
    Bitmap_Set(&g_Bitmap, index, true);
    freeMemory -= 0x1000;
    usedMemory += 0x1000;
}

void pmm_UnreservePage(void* address)
{
    uint64_t index = (uint64_t)address / 4096;
    if (Bitmap_Get(&g_Bitmap, index) == false) return;
    Bitmap_Set(&g_Bitmap, index, false);
    freeMemory += 0x1000;
    reservedMemory -= 0x1000;
}

void pmm_ReservePage(void* address)
{
    uint64_t index = (uint64_t)address / 4096;
    if (Bitmap_Get(&g_Bitmap, index) == true) return;
    Bitmap_Set(&g_Bitmap, index, true);
    freeMemory -= 0x1000;
    reservedMemory += 0x1000;
}

void pmm_FreePages(void* address, uint64_t pagesCount)
{
    for (uint64_t i = 0; i < pagesCount; i++)
    {
        pmm_FreePage((void*)((uint64_t)address + (i * 0x1000)));
    }
}

void pmm_LockPages(void* address, uint64_t pagesCount)
{
    for (uint64_t i = 0; i < pagesCount; i++)
    {
        pmm_LockPage((void*)((uint64_t)address + (i * 0x1000)));
    }
}

void pmm_UnreservePages(void* address, uint64_t pagesCount)
{
    for (uint64_t i = 0; i < pagesCount; i++)
    {
        pmm_UnreservePage((void*)((uint64_t)address + (i * 0x1000)));
    }
}

void pmm_ReservePages(void* address, uint64_t pagesCount)
{
    for (uint64_t i = 0; i < pagesCount; i++)
    {
        pmm_ReservePage((void*)((uint64_t)address + (i * 0x1000)));
    }
}

uint64_t pmm_GetFreeMemory()
{
    return freeMemory;
}

uint64_t pmm_GetUsedMemory()
{
    return usedMemory;
}

uint64_t pmm_GetReservedMemory()
{
    return reservedMemory;
}
