#include "ahci.h"
#include <console.h>
#include <heap.h>
#include <memory.h>
#include <paging.h>
#include <pmm.h>
#include <util.h>

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

#define HBA_RESET (1 << 0)

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define HBA_PxIS_TFES (1 << 30)
#define HBA_PxIS_CCS (1 << 4)

#define AHCI_ENABLE 0x80000000  // AHCI Enable bit in Global Host Control register
#define AHCI_IE 0x00000002      // Interrupt Enable bit in Global Host Control register 

#define AHCI_BIOS_BUSY (1 << 4)
#define AHCI_BIOS_OWNED (1 << 0)
#define AHCI_OS_OWNED (1 << 1)

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
    memset(newBase, 0, 1024);

    void* fisBase = pmm_AllocatePage();
    port->fisBaseAddress = (uint32_t)(uint64_t)fisBase;
    port->fisBaseAddressUpper = (uint32_t)((uint64_t)fisBase >> 32);
    memset(fisBase, 0, 256);

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

bool AHCI_Read(HBAPort* port, uint64_t sector, uint32_t sectorsCount, void* buffer)
{
    uint32_t sectorLow = (uint32_t)sector;
    uint32_t sectorHigh = (uint32_t)(sector >> 32);

    int spin = 0;
    while ((port->taskFileData & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin <= 1000000)
    {
        spin++;
    }

    if (spin >= 1000000)
    {
        return false;
    }

    port->interruptStatus = (uint32_t)-1; // Clear pending interrupt bit

    HBACommandHeader* cmdHeader = (HBACommandHeader*)port->commandListBase;
    cmdHeader->commandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmdHeader->write = 0;
    cmdHeader->prdtLength = 1;

    HBACommandTable* cmdTable = (HBACommandTable*)cmdHeader->commandTableBaseAddress;
    memset(cmdTable, 0, sizeof(HBACommandTable) + (cmdHeader->prdtLength - 1) * sizeof(HBAPRDTEntry));

    cmdTable->prdtEntry[0].dataBaseAddress = (uint32_t)(uint64_t)buffer;
    cmdTable->prdtEntry[0].dataBaseAddressUpper = (uint32_t)((uint64_t)buffer >> 32);
    cmdTable->prdtEntry[0].byteCount = (sectorsCount << 9) - 1;
    cmdTable->prdtEntry[0].interruptOnCompletion = 1;

    FIS_REG_H2D* cmdFIS = (FIS_REG_H2D*)(&cmdTable->commandFIS);
    cmdFIS->fisType = FIS_TYPE_REG_H2D;
    cmdFIS->commandControl = 1;
    cmdFIS->command = ATA_CMD_READ_DMA_EX;

    // Only 48 bits LBA
    cmdFIS->lba0 = (uint8_t)sectorLow;
    cmdFIS->lba1 = (uint8_t)(sectorLow >> 8);
    cmdFIS->lba2 = (uint8_t)(sectorLow >> 16);
    cmdFIS->lba3 = (uint8_t)(sectorLow >> 24);
    cmdFIS->lba4 = (uint8_t)sectorHigh;
    cmdFIS->lba5 = (uint8_t)(sectorHigh >> 8);

    cmdFIS->deviceRegister = 1 << 6; // LBA mode

    // Only 16 bits sectors count
    cmdFIS->countLow = sectorsCount & 0xFF;
    cmdFIS->countHigh = (sectorsCount >> 8) & 0xFF;

    

    // Issue the command
    port->commandIssue = 1;

    // TODO: Use interrupts instead
    while (true)
    {
        if ((port->commandIssue & 1) == 0)
        {
            break;
        }

        if (port->interruptStatus & HBA_PxIS_TFES)
        {
            // Task file error
            return false;
        }
    }

    // Check again
    if (port->interruptStatus & HBA_PxIS_TFES)
    {
        // Task file error
        return false;
    }

    return true;
}

void InitializeAHCI(PCIDevice* device)
{
    // printf("Start init AHCI!\n");
    // PCIGeneralDevice* generalDevice = (PCIGeneralDevice*)malloc(sizeof(PCIGeneralDevice));
    // PCIGetGeneralDevice(device, generalDevice);

    // HBAMemory* abar = (HBAMemory*)(generalDevice->bar[5] & 0xFFFFFFF0);

    // // Enable PCI bus mastering, memory access and interrupts
    // uint32_t command_status = COMBINE_WORD(device->status, device->command);
    // if (!(command_status & (1 << 2)))
    // {
    //     command_status |= (1 << 2);
    // }
    // if (!(command_status & (1 << 1)))
    // {
    //     command_status |= (1 << 1);
    // }
    // if (!(command_status & (1 << 10)))
    // {
    //     command_status |= (1 << 10);
    // }
    // PCIConfigWriteDword(device->bus, device->slot, device->function, 0x04, command_status);

    // ahci* ahciPtr = (ahci*)malloc(sizeof(ahci));
    // memset(ahciPtr, 0, sizeof(ahci));
    // ahciPtr->abar = abar;
    
    // paging_MapMemory(abar, abar);

    // // Do a full HBA reset
    // abar->globalHostControl |= (1 << 0);
    // while (abar->globalHostControl & (1 << 0));

    // abar->globalHostControl |= (AHCI_ENABLE | AHCI_IE);

    // AHCI_ProbePorts(ahciPtr, abar);
}
