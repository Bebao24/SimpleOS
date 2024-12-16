#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>
#include <memory.h>
#include <pmm.h>

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

    for (int i = 0; i < 20; i++)
    {
        void* addr = pmm_AllocatePage();
        printf("Address: 0x%llx\n", (uint64_t)addr);
    }

    printf("Hello World!\n");

    for (;;)
    {
        asm volatile("hlt");
    }
}