#pragma once
#include <stddef.h>

typedef struct
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charSize;
} PSF1_HEADER;

typedef struct
{
	PSF1_HEADER* fontHeader;
	void* glyphBuffer;
} PSF1_FONT;

typedef struct
{
	void* BaseAddress;
	size_t BufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int PixelsPerScanLine;
} GOP_Framebuffer_t;

typedef struct
{
    GOP_Framebuffer_t* fb;
	PSF1_FONT* font;
} BootInfo;