#pragma once
#include <efiMemory.h>
#include <stdint.h>

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap, uint64_t mMapEntries, uint64_t mDescriptorSize);

