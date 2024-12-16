#include "bitmap.h"

bool Bitmap_Get(Bitmap* bitmap, uint64_t index)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8; // The remainder
    uint8_t bitIndexer = 0b10000000 >> bitIndex;

    if ((bitmap->bitmapBuffer[byteIndex] & bitIndexer) > 0)
    {
        return true;
    }

    return false;
}

void Bitmap_Set(Bitmap* bitmap, uint64_t index, bool value)
{
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8; // The remainder
    uint8_t bitIndexer = 0b10000000 >> bitIndex;

    bitmap->bitmapBuffer[byteIndex] &= ~(bitIndexer); // Unset that bit
    if (value)
    {
        bitmap->bitmapBuffer[byteIndex] |= bitIndexer; // Set that bit
    }
}
