#include <bootInfo.h>
#include <stdint.h>
#include <framebuffer.h>
#include <console.h>
#include <bitmap.h>
#include <memory.h>

void kmain(BootInfo* bootInfo)
{
    InitializeFb(bootInfo->fb, bootInfo->font);
    InitializeConsole();
    clearScreen();

    uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mDescriptorSize;

    uint8_t testBuffer[20];
    memset(testBuffer, 0, sizeof(testBuffer));

    Bitmap testBitmap;
    testBitmap.bitmapBuffer = &testBuffer[0];
    Bitmap_Set(&testBitmap, 1, true);
    Bitmap_Set(&testBitmap, 3, true);
    Bitmap_Set(&testBitmap, 5, true);
    Bitmap_Set(&testBitmap, 8, true);
    Bitmap_Set(&testBitmap, 11, true);
    Bitmap_Set(&testBitmap, 12, true);

    for (int i = 0; i < 15; i++)
    {
        printf(Bitmap_Get(&testBitmap, i) ? "true" : "false");
        putc('\n');
    }

    for (;;)
    {
        asm volatile("hlt");
    }
}