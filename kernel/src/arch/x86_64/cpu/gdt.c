#include "gdt.h"
#include <stdint.h>

typedef struct
{
    uint16_t Limit;
    uint16_t BaseLow;
    uint8_t BaseMiddle;
    uint8_t Access;
    uint8_t Flags;
    uint8_t BaseHigh;
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t Limit;
    uint64_t Offset;
} __attribute__((packed)) GDTDescriptor;

typedef struct
{
    GDTEntry Null;
    GDTEntry KernelCode;
    GDTEntry KernelData;
    GDTEntry UserNull;
    GDTEntry UserCode;
    GDTEntry UserData;
} __attribute__((packed))
__attribute__((aligned(0x1000))) GDT;

__attribute__((aligned(0x1000)))
GDT DefaultGDT = {
    { 0, 0, 0, 0x00, 0x00, 0 }, // NULL
    { 0, 0, 0, 0x9a, 0xa0, 0 }, // Kernel code segment
    { 0, 0, 0, 0x92, 0xa0, 0 }, // kernel data segment
    { 0, 0, 0, 0x00, 0x00, 0 }, // User Null
    { 0, 0, 0, 0xFA, 0xa0, 0 }, // User code segment
    { 0, 0, 0, 0xF2, 0xa0, 0 } // User data segment
};

extern void LoadGDT(GDTDescriptor* descriptor);

GDTDescriptor g_GDTDescriptor;

void InitializeGDT()
{
    g_GDTDescriptor.Limit = sizeof(GDT) - 1;
    g_GDTDescriptor.Offset = (uint64_t)&DefaultGDT;
    LoadGDT(&g_GDTDescriptor);
}
