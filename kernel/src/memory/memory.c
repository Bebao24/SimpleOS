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

