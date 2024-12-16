#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    size_t bitmapSize;
    uint8_t* bitmapBuffer;
} Bitmap;

bool Bitmap_Get(Bitmap* bitmap, uint64_t index);
void Bitmap_Set(Bitmap* bitmap, uint64_t index, bool value);

