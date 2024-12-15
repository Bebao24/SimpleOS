#include <bootInfo.h>
#include <stdint.h>

void putChar(GOP_Framebuffer_t* fb, PSF1_FONT* font, uint32_t color, char c, uint32_t x, uint32_t y)
{
    uint32_t* pixelPtr = (uint32_t*)fb->BaseAddress;

    char* fontPtr = font->glyphBuffer + (c * font->fontHeader->charSize);

    for (uint32_t yy = y; yy < y + 16; yy++)
    {
        for (uint32_t xx = x; xx < x + 16; xx++)
        {
            if ((*fontPtr & (0b10000000 >> (xx - x))) > 0)
            {
                *(uint32_t*)(pixelPtr + xx + (yy * fb->PixelsPerScanLine)) = color;
            }
        }
        fontPtr++;
    }
}

void kmain(BootInfo* bootInfo)
{
    putChar(bootInfo->fb, bootInfo->font, 0xFFFFFFFF, 'H', 50, 50);   

    for (;;)
    {
        asm volatile("hlt");
    }
}