#include <stddef.h>

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
} BootInfo;