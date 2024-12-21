#include "idt.h"

typedef struct
{
    uint16_t OffsetLow;
    uint16_t SegmentSelector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t OffsetMiddle;
    uint32_t OffsetHigh;
    uint32_t Reserved;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t Limit;
    uint64_t Offset;
} __attribute__((packed)) IDTDescriptor;

IDTEntry g_IDTEntries[256] = { 0 };
IDTDescriptor g_IDTDescriptor;

void IDTSetGate(int interrupt, uint64_t handler, uint8_t flags)
{
    g_IDTEntries[interrupt].OffsetLow = (uint16_t)handler;
    g_IDTEntries[interrupt].SegmentSelector = 0x08; // Kernel code segment
    g_IDTEntries[interrupt].ist = 0;
    g_IDTEntries[interrupt].Reserved = 0;
    g_IDTEntries[interrupt].attributes = flags;
    g_IDTEntries[interrupt].OffsetMiddle = (uint16_t)(handler >> 16);
    g_IDTEntries[interrupt].OffsetHigh = (uint32_t)(handler >> 32);
}

void InitializeIDT()
{
    g_IDTDescriptor.Limit = sizeof(g_IDTEntries) - 1;
    g_IDTDescriptor.Offset = (uint64_t)&g_IDTEntries;
    asm volatile("lidt %0" : : "m"(g_IDTDescriptor) : "memory");
}
