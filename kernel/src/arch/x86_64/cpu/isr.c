#include "isr.h"
#include <idt.h>
#include <stddef.h>
#include <console.h>
#include <system.h>

ISRHandler g_ISRHandler[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void InitializeISR()
{
    // TODO: Remap PIC

    for (int i = 0; i < 32; i++)
    {
        IDTSetGate(i, (uint64_t)isr_stub_table[i], IDT_TA_InterruptGate);
    }

    asm volatile("sti");
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    g_ISRHandler[interrupt] = handler;
}

void interrupt_handler(cpu_registers_t* cpu)
{
    if (g_ISRHandler[cpu->interrupt_number] != NULL)
    {
        g_ISRHandler[cpu->interrupt_number](cpu);
    }
    else if (cpu->interrupt_number >= 32) // IRQs and syscall
    {
        printf("Unhandled interrupt: %d!\n", cpu->interrupt_number);
    }
    else // Exceptions
    {
        clearScreen();

        printf("Exception: %s\n", g_Exceptions[cpu->interrupt_number]);

        printf("Interrupt number: %d  Error code: %d\n", cpu->interrupt_number, cpu->error_code);
        printf("KERNEL PANIC!!!\n");

        panic();
    }
}
