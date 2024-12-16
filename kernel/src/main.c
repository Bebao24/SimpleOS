#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>
#include <memory.h>

void kmain(BootInfo* bootInfo)
{
    InitializeFb(bootInfo->fb, bootInfo->font);
    InitializeConsole();
    clearScreen();

    uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mDescriptorSize;
    printf("Total memory: %llu KB\n", GetMemorySize(bootInfo->mMap, mMapEntries, bootInfo->mDescriptorSize) / 1024);

    for (;;)
    {
        asm volatile("hlt");
    }
}