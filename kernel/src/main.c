#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>

void kmain(BootInfo* bootInfo)
{
    InitializeFb(bootInfo->fb, bootInfo->font);
    fb_clearScreen(COLOR(0, 0, 255));
    
    fb_putChar(50, 50, 'G', COLOR(255, 255, 255));

    for (;;)
    {
        asm volatile("hlt");
    }
}