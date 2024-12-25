#pragma once
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_BUSES 256
#define PCI_MAX_DEVICES 32
#define PCI_MAX_FUNCTIONS 8

typedef struct
{
    uint16_t vendorId;
    uint16_t deviceId;
    uint16_t command;
    uint16_t status;

    uint8_t revisionId;
    uint8_t progIF; // Program interface
    uint8_t subclass;
    uint8_t class;

    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
} PCIDevice;

typedef struct PCI PCI;

struct PCI
{
    PCI* next;

    uint8_t bus;
    uint8_t slot;
    uint8_t function;

    uint16_t vendorId;
    uint16_t deviceId;
    uint8_t subclass;
    uint8_t class;
};

extern PCI* firstPCI;

uint16_t PCIConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
int PCICheckDevice(uint8_t bus, uint8_t slot, uint8_t function);
void PCIGetDevice(PCIDevice* deviceOut, uint8_t bus, uint8_t slot, uint8_t function);

void InitializePCI();
