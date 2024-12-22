#pragma once
#include <stdint.h>

#define IDT_TA_InterruptGate    0b10001110
#define IDT_TA_CallGate         0b10001100
#define IDT_TA_TrapGate         0b10001111

void InitializeIDT();
void IDTSetGate(int interrupt, uint64_t handler, uint8_t flags);
