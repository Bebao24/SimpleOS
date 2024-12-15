#pragma once
#include <bootInfo.h>
#include <stdint.h>

#define COLOR(r, g, b) ((r << 16) | (g << 8) | (b))

extern uint64_t fb_Width;
extern uint64_t fb_Height;
extern uint64_t fb_Pitch;

void InitializeFb(GOP_Framebuffer_t* framebuffer, PSF1_FONT* font);

void fb_putPixel(uint32_t x, uint32_t y, uint32_t color);
void fb_clearScreen(uint32_t color);
void fb_drawRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_putChar(uint32_t x, uint32_t y, char c, uint32_t color);
