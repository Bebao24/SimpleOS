#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 0x1000

#define PGSHIFT_PDP 39
#define PGSHIFT_PD 30
#define PGSHIFT_PT 21
#define PGSHIFT_P 12
#define PGMASK_ENTRY 0x1ff

#define PDP_I(a) (((a) >> PGSHIFT_PDP) & PGMASK_ENTRY)
#define PD_I(a) (((a) >> PGSHIFT_PD) & PGMASK_ENTRY)
#define PT_I(a) (((a) >> PGSHIFT_PT) & PGMASK_ENTRY)
#define P_I(a) (((a) >> PGSHIFT_P) & PGMASK_ENTRY)

// Using a bitfield is probably not the best option...
typedef struct
{
    bool Present : 1;
    bool ReadWrite : 1;
    bool UserSuper : 1;
    bool WriteThrough : 1;
    bool CachedDisabled : 1;
    bool Accessed : 1;
    bool Reserved0 : 1;
    bool LargerPages : 1;
    bool Reserved1 : 1;
    uint8_t Avaliable : 3;
    uint64_t Address : 40;
} PageDirectoryEntry;

typedef struct
{
    PageDirectoryEntry entries[512];
} __attribute__((aligned(PAGE_SIZE))) PageTable;

void InitializePaging(PageTable* PML4);

void paging_MapMemory(void* virtualAddr, void* physicalAddr);
