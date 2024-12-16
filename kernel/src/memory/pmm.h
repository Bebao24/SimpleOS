#pragma once
#include <efiMemory.h>
#include <stdint.h>

void InitializePMM(EFI_MEMORY_DESCRIPTOR* mMap, uint64_t mMapSize, uint64_t mDescriptorSize);

void* pmm_AllocatePage();

void pmm_FreePage(void* address);
void pmm_LockPage(void* address);
void pmm_UnreservePage(void* address);
void pmm_ReservePage(void* address);

void pmm_FreePages(void* address, uint64_t pagesCount);
void pmm_LockPages(void* address, uint64_t pagesCount);
void pmm_UnreservePages(void* address, uint64_t pagesCount);
void pmm_ReservePages(void* address, uint64_t pagesCount);

uint64_t pmm_GetFreeMemory();
uint64_t pmm_GetUsedMemory();
uint64_t pmm_GetReservedMemory();
