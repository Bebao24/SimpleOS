#pragma once
#include <cpu.h>
#include <stdbool.h>

#define LeftShift 0x2A
#define RightShift 0x36
#define Enter 0x1C
#define BackSpace 0x0E
#define Spacebar 0x39

typedef struct
{
    bool uppercase;
    char key;
} key_info_t;

void InitializeKeyboard();

char TranslateScancode(uint8_t scancode, bool uppercase);
void IRQ_KeyboardHandler(cpu_registers_t* cpu);

char getKey();

