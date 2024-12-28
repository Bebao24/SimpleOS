#pragma once
#include <acpi.h>

#define PCI_MAX_BUSES 256
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

typedef struct
{
    uint16_t VendorId;
    uint16_t DeviceId;
    uint16_t Command;
    uint16_t Status;
    uint8_t RevisionId;
    uint8_t ProgIF; // Program interface
    uint8_t Subclass;
    uint8_t Class;
    uint8_t CacheLineSize;
    uint8_t LatencyTimer;
    uint8_t HeaderType;
    uint8_t BIST;    
} PCIDevice;

typedef struct
{
    PCIDevice deviceHeader;
    uint32_t bar[6];
    uint32_t CardBusCISPtr;
    uint16_t SubSystemVendorId;
    uint16_t SubSystemId;
    uint32_t ExpansionROMBaseAddress;
    uint8_t CapabilitiesPtr;
    uint8_t Reserved0;
    uint16_t Reserved1;
    uint32_t Reserved2;
    uint8_t InterruptLine;
    uint8_t InterruptPin;
    uint8_t MinGrant;
    uint8_t MaxLatency;
} PCIGeneralDevice;

typedef struct PCI PCI;

struct PCI
{
    PCI* next;
    uint16_t VendorId, DeviceId;
    uint8_t Subclass, Class;
};

extern PCI* firstPCI;

void InitializePCI(MCFGHeader* mcfg);
