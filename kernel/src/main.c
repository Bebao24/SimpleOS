#include <bootInfo.h>

void kmain(BootInfo* bootInfo)
{
    unsigned int y = 50;
	unsigned int BPP = 4;
	for (unsigned int x = 0; x < bootInfo->fb->width * BPP / 2; x += BPP)
	{
		*(unsigned int*)(x + (y * bootInfo->fb->PixelsPerScanLine * BPP) + bootInfo->fb->BaseAddress) = 0xFFFFFFFF;
	}

    for (;;)
    {
        asm volatile("hlt");
    }
}