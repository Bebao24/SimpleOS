#pragma once
#include <cpu.h>

typedef void (*IRQHandler)(cpu_registers_t* cpu);

void InitializeIRQ();
void IRQ_RegisterHandler(int irq, IRQHandler handler);

