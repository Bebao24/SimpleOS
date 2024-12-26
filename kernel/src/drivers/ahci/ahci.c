#include "ahci.h"
#include <console.h>
#include <heap.h>
#include <memory.h>
#include <paging.h>
#include <pmm.h>

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

int AHCI_CheckPortType(HBAPort* port)
{
    uint32_t sataStatus = port->sataStatus;

    uint8_t ipm = (sataStatus >> 8) & 0x0F;
    uint8_t det = sataStatus & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) return AHCI_DEV_NONE;
    if (ipm != HBA_PORT_IPM_ACTIVE) return AHCI_DEV_NONE;

    switch (port->signature)
    {
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        default:
            return AHCI_DEV_NONE;
    }
}

void AHCI_ProbePorts(ahci* ahciPtr, HBAMemory* abar)
{
    uint32_t portsImplemented = abar->portsImplemented;

    for (int i = 0; i < 32; i++)
    {
        if (portsImplemented & 1)
        {
            int portType = AHCI_CheckPortType(&abar->ports[i]);

            switch (portType)
            {
                case AHCI_DEV_SATA:
                    printf("Drive found: SATA\n");
                    AHCI_PortRebase(ahciPtr, &ahciPtr->abar->ports[i], i);
                    break;
                case AHCI_DEV_SATAPI:
                    printf("(unsupported) Drive found: SATAPI\n");
                    break;
                case AHCI_DEV_PM:
                    printf("(unsupported) Drive found: PM\n");
                    break;
                case AHCI_DEV_SEMB:
                    printf("(unsupported) Drive found: SEMB\n");
                    break;
            }
        }

        portsImplemented >>= 1;
    }
}

void AHCI_StopCmd(HBAPort* port)
{
    port->cmdSts &= ~HBA_PxCMD_ST;
    port->cmdSts &= ~HBA_PxCMD_FRE;

    while (true)
    {
        if (port->cmdSts & HBA_PxCMD_FR) continue;
        if (port->cmdSts & HBA_PxCMD_CR) continue;

        break;
    }
}

void AHCI_StartCmd(HBAPort* port)
{
    while (port->cmdSts & HBA_PxCMD_CR);

    port->cmdSts |= HBA_PxCMD_FRE;
    port->cmdSts |= HBA_PxCMD_ST;
}

void AHCI_PortRebase(ahci* ahciPtr, HBAPort* port, int portNumber)
{
    AHCI_StopCmd(port); // Stop command engine

    // Wasted a little bit of memory
    void* newBase = pmm_AllocatePage();
    port->commandListBase = (uint32_t)(uint64_t)newBase;
    port->commandListBaseUpper = (uint32_t)((uint64_t)newBase >> 32);
    memset((void*)port->commandListBase, 0, 1024);

    void* fisBase = pmm_AllocatePage();
    port->fisBaseAddress = (uint32_t)(uint64_t)fisBase;
    port->fisBaseAddressUpper = (uint32_t)((uint64_t)fisBase >> 32);
    memset((void*)port->fisBaseAddress, 0, 256);

    HBACommandHeader* cmdHeader = (HBACommandHeader*)((uint64_t)port->commandListBase + ((uint64_t)port->commandListBaseUpper << 32));

    for (int i = 0; i < 32; i++)
    {
        cmdHeader[i].prdtLength = 8;

        void* cmdTableAddress = pmm_AllocatePage();
        uint64_t address = (uint64_t)cmdTableAddress + (i << 8);
        cmdHeader[i].commandTableBaseAddress = (uint32_t)(uint64_t)address;
        cmdHeader[i].commandTableBaseAddressUpper = (uint32_t)((uint64_t)address >> 32);
        memset(cmdTableAddress, 0, 256);
    }

    AHCI_StartCmd(port); // Start command engine
    ahciPtr->activePorts |= (1 << portNumber);
}

void InitializeAHCI(PCIDevice* device)
{
    PCIGeneralDevice* generalDevice = (PCIGeneralDevice*)malloc(sizeof(PCIGeneralDevice));
    PCIGetGeneralDevice(device, generalDevice);

    ahci* ahciPtr = (ahci*)malloc(sizeof(ahci));
    memset(ahciPtr, 0, sizeof(ahci));

    uint32_t base = generalDevice->bar[5] & 0xFFFFFFF0; // Align to a 16 bytes boundary

    paging_MapMemory(base, base);

    HBAMemory* abar = (HBAMemory*)base;

    ahciPtr->abar = abar;

    AHCI_ProbePorts(ahciPtr, abar);
}
