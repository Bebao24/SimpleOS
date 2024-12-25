#include "ahci.h"
#include <console.h>
#include <heap.h>
#include <memory.h>
#include <paging.h>

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

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
                    break;
                case AHCI_DEV_SATAPI:
                    printf("Drive found: SATAPI\n");
                    break;
                case AHCI_DEV_PM:
                    printf("Drive found: PM\n");
                    break;
                case AHCI_DEV_SEMB:
                    printf("Drive found: SEMB\n");
                    break;
            }
        }

        portsImplemented >>= 1;
    }
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
