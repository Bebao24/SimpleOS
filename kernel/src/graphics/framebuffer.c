#include "framebuffer.h"
#include <stddef.h>

uint32_t* fb = NULL;
PSF1_FONT* g_Font = NULL;

uint64_t fb_Width;
uint64_t fb_Height;
uint64_t fb_Pitch;

void InitializeFb(GOP_Framebuffer_t* framebuffer, PSF1_FONT* font)
{
    fb = (uint32_t*)framebuffer->BaseAddress;
    fb_Width = framebuffer->width;
    fb_Height = framebuffer->height;
    fb_Pitch = framebuffer->PixelsPerScanLine * 4;

    g_Font = font;
}

void fb_putPixel(uint32_t x, uint32_t y, uint32_t color)
{
    uint32_t offset = (x * 4) + (y * fb_Pitch);
    fb[offset / 4] = color;
}

void fb_clearScreen(uint32_t color)
{
    for (uint32_t y = 0; y < fb_Height; y++)
    {
        for (uint32_t x = 0; x < fb_Width; x++)
        {
            uint32_t offset = (x * 4) + (y * fb_Pitch);
            fb[offset / 4] = color;
        }
    }
}

void fb_drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    for (uint32_t yy = 0; yy < h; yy++)
    {
        for (uint32_t xx = 0; xx < w; xx++)
        {
            uint32_t offset = ((x + xx) * 4) + ((y + yy) * fb_Pitch);
            fb[offset / 4] = color;
        }
    }
}

void fb_putChar(uint32_t x, uint32_t y, char c, uint32_t color)
{
    uint8_t* fontPtr = g_Font->glyphBuffer + (c * g_Font->fontHeader->charSize);

    for (uint32_t yy = y; yy < y + 16; yy++)
    {
        for (uint32_t xx = x; xx < x + 8; xx++)
        {
            if ((*fontPtr & (0b10000000 >> (xx - x))) > 0)
            {
                fb_putPixel(xx, yy, color);
            }
        }
        fontPtr++;
    }
}

