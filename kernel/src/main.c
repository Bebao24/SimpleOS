#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>
#include <memory.h>
#include <pmm.h>
#include <paging.h>
#include <heap.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <pic.h>
#include <system.h>
#include <keyboard.h>

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

#define HEAP_ADDRESS ((void*)0x0000100000000000)

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

    InitializeHeap(HEAP_ADDRESS, 0x10);

    InitializeGDT();
    InitializeIDT();
    InitializeISR();
    InitializeIRQ();

    PIC_Mask(0);
    InitializeKeyboard();

    printf("Hello World!\n");

    while (true)
    {
        char key = getKey();

        if (key == '\r')
        {
            putc('\n');
            continue;
        }

        putc(key);
    }

    for (;;)
    {
        asm volatile("hlt");
    }
}