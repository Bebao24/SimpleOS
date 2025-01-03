#pragma once
#include <stdint.h>
#include <pci.h>
#include <stdbool.h>

#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EX 0x35

#define AHCI_DEV_NONE 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

typedef struct
{
    uint32_t commandListBase;
    uint32_t commandListBaseUpper;
    uint32_t fisBaseAddress;
    uint32_t fisBaseAddressUpper;
    uint32_t interruptStatus;
    uint32_t interruptEnable;
    uint32_t cmdSts;
    uint32_t reserved0;
    uint32_t taskFileData;
    uint32_t signature;
    uint32_t sataStatus;
    uint32_t sataControl;
    uint32_t sataError;
    uint32_t sataActive;
    uint32_t commandIssue;
    uint32_t sataNotification;
    uint32_t fisSwitchControl;
    uint32_t reserved1[11];
    uint32_t vendor[4];
} HBAPort;

typedef struct
{
    uint32_t hostCapability;
    uint32_t globalHostControl;
    uint32_t interruptStatus;
    uint32_t portsImplemented;
    uint32_t version;
    uint32_t cccControl;
    uint32_t cccPorts;
    uint32_t enclosureManagementLocation;
    uint32_t enclosureManagementControl;
    uint32_t hostCapabilitiesExtended;
    uint32_t biosHandoffCtrlSts;
    uint8_t reserved0[0x74];
    uint8_t vendor[0x60];
    HBAPort ports[1];
} HBAMemory;

typedef struct
{
    uint8_t commandFISLength : 5;
    uint8_t atapi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;

    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clearBusy : 1;
    uint8_t reserved0 : 1;
    uint8_t portMultiplier : 4;

    uint16_t prdtLength;
    uint32_t prdtCount;
    uint32_t commandTableBaseAddress;
    uint32_t commandTableBaseAddressUpper;
    uint32_t reserved1[4];
} HBACommandHeader;

typedef struct
{
    uint32_t dataBaseAddress;
    uint32_t dataBaseAddressUpper;
    uint32_t reserved0;

    uint32_t byteCount : 22;
    uint32_t reserved1 : 9;
    uint32_t interruptOnCompletion : 1;
} HBAPRDTEntry;

typedef struct
{
    uint8_t commandFIS[64];
    uint8_t atapiCommand[16];
    uint8_t reserved[48];

    HBAPRDTEntry prdtEntry[1];
} HBACommandTable;

typedef struct
{
    uint8_t fisType;

    uint8_t portMultiplier : 4;
    uint8_t reserved0 : 3;
    uint8_t commandControl : 1;

    uint8_t command;
    uint8_t featureLow;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t deviceRegister;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureHigh;

    uint8_t countLow;
    uint8_t countHigh;
    uint8_t isoCommandCompletion;
    uint8_t control;

    uint8_t reserved1[4];
} FIS_REG_H2D;

typedef struct
{
    uint32_t activePorts; // 32 ports -> 32 bits
    HBAMemory* abar;
} ahci;

void InitializeAHCI(PCIDevice* device);
void AHCI_ProbePorts(ahci* ahciPtr, HBAMemory* abar);
int AHCI_CheckPortType(HBAPort* port);
void AHCI_PortRebase(ahci* ahciPtr, HBAPort* port, int portNumber);

bool AHCI_Read(HBAPort* port, uint64_t sector, uint32_t sectorsCount, void* buffer);

