#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>

void kmain(BootInfo* bootInfo)
{
    InitializeFb(bootInfo->fb, bootInfo->font);
    InitializeConsole();
    clearScreen();

    printf("Hello World from printf!!!!\n");
    printf("Testing: 0x%x\n", 0x123);

    for (;;)
    {
        asm volatile("hlt");
    }
}