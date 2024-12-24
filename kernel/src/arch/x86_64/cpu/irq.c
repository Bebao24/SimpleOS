#include "irq.h"
#include <pic.h>
#include <system.h>
#include <stddef.h>
#include <isr.h>
#include <console.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void DefaultIRQ_Handler(cpu_registers_t* cpu)
{
    int irq = cpu->interrupt_number - PIC_REMAP_OFFSET;

    if (g_IRQHandlers[irq] != NULL)
    {
        g_IRQHandlers[irq](cpu);
    }
    else
    {
        printf("Unhandled IRQ: %d\n", irq);
    }

    // Send EOI
    PIC_SendEOI(irq);
}

void InitializeIRQ()
{
    PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    for (int i = 0; i < 16; i++)
    {
        ISR_RegisterHandler(PIC_REMAP_OFFSET + i, DefaultIRQ_Handler);
    }

    // Re-enable interrupts
    asm volatile("sti");
}

void IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}
