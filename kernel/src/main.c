#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>
#include <memory.h>
#include <pmm.h>
#include <paging.h>

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

void kmain(BootInfo* bootInfo)
{
    InitializeFb(bootInfo->fb, bootInfo->font);
    InitializeConsole();
    clearScreen();

    uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mDescriptorSize;

    InitializePMM(bootInfo->mMap, bootInfo->mMapSize, bootInfo->mDescriptorSize);

    uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
    uint64_t kernelPages = (uint64_t)kernelSize / 0x1000 + 1;
    pmm_LockPages(&_KernelStart, kernelPages);

    PageTable* PML4 = (PageTable*)pmm_AllocatePage();
    memset(PML4, 0, 0x1000);

    InitializePaging(PML4);

    // Identity mapping all physical memory
    for (uint64_t i = 0; i < GetMemorySize(bootInfo->mMap, mMapEntries, bootInfo->mDescriptorSize); i += 0x1000)
    {
        paging_MapMemory((void*)i, (void*)i);
    }

    uint64_t fbBase = (uint64_t)bootInfo->fb->BaseAddress;
    uint64_t fbSize = (uint64_t)bootInfo->fb->BufferSize + 0x1000;
    for (uint64_t i = fbBase; i < fbBase + fbSize; i += 0x1000)
    {
        paging_MapMemory((void*)i, (void*)i);
    }

    // Pass the new page table to the CPU
    asm volatile("mov %0, %%cr3" : : "r"(PML4));

    printf("Hello World!\n");

    paging_MapMemory((void*)0x600000000, (void*)0x80000);
    uint64_t* test = 0x600000000;
    *test = 69;
    printf("test: %d\n", *test);

    for (;;)
    {
        asm volatile("hlt");
    }
}