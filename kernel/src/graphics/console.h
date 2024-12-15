#pragma once
#include <framebuffer.h>

#define BACKGROUND_COLOR COLOR(0, 0, 255)
#define FOREGROUND_COLOR COLOR(255, 255, 255)

void InitializeConsole();

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

void clearScreen();
void putc(char c);
void puts(const char* string);
void printf(const char* fmt, ...);

