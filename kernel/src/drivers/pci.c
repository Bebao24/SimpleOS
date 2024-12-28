#include "pci.h"
#include <paging.h>
#include <console.h>
#include <linked_list.h>

PCI* firstPCI;

void EnumerateFunction(uint64_t deviceAddress, uint64_t function)
{
    uint64_t offset = function << 12;

    uint64_t functionAddress = deviceAddress + offset;
    paging_MapMemory((void*)functionAddress, (void*)functionAddress);

    PCIDevice* pciDevice = (PCIDevice*)functionAddress;

    if (pciDevice->DeviceId == 0) return;
    if (pciDevice->DeviceId == 0xFFFF) return;

    PCI* target = (PCI*)LinkedListAllocate((void**)&firstPCI, sizeof(PCI));
    target->VendorId = pciDevice->VendorId;
    target->DeviceId = pciDevice->DeviceId;
    target->Subclass = pciDevice->Subclass;
    target->Class = pciDevice->Class;
}

void EnumerateDevice(uint64_t busAddress, uint64_t device)
{
    uint64_t offset = device << 15;

    uint64_t deviceAddress = busAddress + offset;
    paging_MapMemory((void*)deviceAddress, (void*)deviceAddress);

    PCIDevice* pciDevice = (PCIDevice*)deviceAddress;

    if (pciDevice->DeviceId == 0) return;
    if (pciDevice->DeviceId == 0xFFFF) return;

    for (uint64_t function = 0; function < PCI_MAX_FUNCTIONS; function++)
    {
        EnumerateFunction(deviceAddress, function);
    }
}

void EnumerateBus(uint64_t baseAddress, uint64_t bus)
{
    uint64_t offset = bus << 20;

    uint64_t busAddress = baseAddress + offset;
    paging_MapMemory((void*)busAddress, (void*)busAddress);

    PCIDevice* pciDevice = (PCIDevice*)busAddress;

    if (pciDevice->DeviceId == 0) return;
    if (pciDevice->DeviceId == 0xFFFF) return;

    for (uint64_t device = 0; device < PCI_MAX_DEVICES; device++)
    {
        EnumerateDevice(busAddress, device);
    }
}

void InitializePCI(MCFGHeader* mcfg)
{
    int entries = (mcfg->header.Length - sizeof(MCFGHeader)) / sizeof(DeviceConfig);

    for (int i = 0; i < entries; i++)
    {
        DeviceConfig* newDeviceConfig = (DeviceConfig*)((uint64_t)mcfg + sizeof(MCFGHeader) + (sizeof(DeviceConfig) * i));
        
        for (uint64_t bus = newDeviceConfig->StartBus; bus < newDeviceConfig->EndBus; bus++)
        {
            EnumerateBus(newDeviceConfig->PCIBaseAddress, bus);
        }
    }
}
