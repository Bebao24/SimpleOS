#include "pci.h"
#include <system.h>
#include <console.h>
#include <util.h>
#include <heap.h>
#include <linked_list.h>

PCI* firstPCI;

uint16_t PCIConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address;

    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunction = (uint32_t)function;
    uint16_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunction << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    x64_outl(PCI_CONFIG_ADDRESS, address);

    // Read in the data
    tmp = (uint16_t)((x64_inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

int PCICheckDevice(uint8_t bus, uint8_t slot, uint8_t function)
{
    uint16_t vendorId = PCIConfigReadWord(bus, slot, function, 0x00);

    return !(vendorId == 0xFFFF || !vendorId);
}

void PCIGetDevice(PCIDevice* deviceOut, uint8_t bus, uint8_t slot, uint8_t function)
{
    deviceOut->vendorId = PCIConfigReadWord(bus, slot, function, 0x00);
    deviceOut->deviceId = PCIConfigReadWord(bus, slot, function, 0x02);

    deviceOut->command = PCIConfigReadWord(bus, slot, function, 0x04);
    deviceOut->status = PCIConfigReadWord(bus, slot, function, 0x06);

    // Since our read function reads 2 bytes, we need to export each byte
    uint16_t revisionId_ProgIF = PCIConfigReadWord(bus, slot, function, 0x08);
    deviceOut->revisionId = EXPORT_BYTE(revisionId_ProgIF, true);
    deviceOut->progIF = EXPORT_BYTE(revisionId_ProgIF, false);

    uint16_t subclass_class = PCIConfigReadWord(bus, slot, function, 0x0a);
    deviceOut->subclass = EXPORT_BYTE(subclass_class, true);
    deviceOut->class = EXPORT_BYTE(subclass_class, false);

    uint16_t cacheLineSize_latencyTimer = PCIConfigReadWord(bus, slot, function, 0x0c);
    deviceOut->cacheLineSize = EXPORT_BYTE(cacheLineSize_latencyTimer, true);
    deviceOut->latencyTimer = EXPORT_BYTE(cacheLineSize_latencyTimer, false);

    uint16_t headerType_BIST = PCIConfigReadWord(bus, slot, function, 0x0e);
    deviceOut->headerType = EXPORT_BYTE(headerType_BIST, true);
    deviceOut->BIST = EXPORT_BYTE(headerType_BIST, false);
}

void InitializePCI()
{
    PCIDevice* device = (PCIDevice*)malloc(sizeof(PCIDevice));

    for (uint16_t bus = 0; bus < PCI_MAX_BUSES; bus++)
    {
        for (uint8_t slot = 0; slot < PCI_MAX_DEVICES; slot++)
        {
            for (uint8_t function = 0; function < PCI_MAX_FUNCTIONS; function++)
            {
                if (!PCICheckDevice(bus, slot, function))
                {
                    continue;
                }

                PCIGetDevice(device, bus, slot, function);

                PCI* target = (PCI*)LinkedListAllocate((void**)(&firstPCI), sizeof(PCI));
                target->bus = bus;
                target->slot = slot;
                target->function = function;

                target->vendorId = device->vendorId;
                target->deviceId = device->deviceId;
                target->subclass = device->subclass;
                target->class = device->class;
            }
        }
    }

    free(device);
}
