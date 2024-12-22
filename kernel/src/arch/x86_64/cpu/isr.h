#pragma once
#include <cpu.h>

typedef void (*ISRHandler)(cpu_registers_t* cpu);

void InitializeISR();
void ISR_RegisterHandler(int interrupt, ISRHandler handler);

extern void* isr_stub_table[];

