#include "memory.h"

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap, uint64_t mMapEntries, uint64_t mDescriptorSize)
{
    static uint64_t totalMemoryBytes = 0; // Use static variable so that we only need to calculate once
    if (totalMemoryBytes > 0) return totalMemoryBytes;

    for (uint64_t i = 0; i < mMapEntries; i++)
    {
        EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mDescriptorSize));
        totalMemoryBytes += descriptor->numPages * 0x1000;
    }

    return totalMemoryBytes;
}

void* memcpy(void* dst, const void* src, size_t num)
{
    uint8_t* u8Dst = (uint8_t *)dst;
    const uint8_t* u8Src = (const uint8_t *)src;

    for (size_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void* memset(void * ptr, int value, size_t num)
{
    uint8_t* u8Ptr = (uint8_t *)ptr;

    for (size_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (size_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}
